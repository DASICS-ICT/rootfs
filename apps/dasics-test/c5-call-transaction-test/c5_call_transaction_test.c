// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

static struct dasics_call_frame c5_frame;
static unsigned long c5_parent_data[2];
static unsigned long c5_child_ro[2];
static unsigned long c5_child_rw[2];

static noinline void c5_target_a(void)
{
}

static noinline void c5_target_b(void)
{
}

static const struct dasics_code_range c5_code_ranges[] = {
	{ .base = (unsigned long)c5_target_a, .size = 4 },
	{ .base = (unsigned long)c5_target_b, .size = 4 },
};

static const struct dasics_compartment c5_compartment = {
	.code_ranges = c5_code_ranges,
	.nr_code_ranges = ARRAY_SIZE(c5_code_ranges),
};

static const struct dasics_region c5_regions[] = {
	{
		.base = (unsigned long)c5_child_ro,
		.size = sizeof(c5_child_ro),
		.perms = DASICS_REGION_READ,
	},
	{
		.base = (unsigned long)c5_child_rw,
		.size = sizeof(c5_child_rw),
		.perms = DASICS_REGION_READ | DASICS_REGION_WRITE,
	},
};

static const struct dasics_call_policy c5_policy = {
	.callee = (struct dasics_compartment *)&c5_compartment,
	.target = c5_target_a,
	.regions = c5_regions,
	.nr_regions = ARRAY_SIZE(c5_regions),
	.stack_size = 512,
};

static bool c5_hw_states_equal(const struct dasics_hw_state *left,
			       const struct dasics_hw_state *right)
{
	return left->libcfg == right->libcfg &&
	       left->jumpcfg == right->jumpcfg &&
	       left->dmaincall == right->dmaincall &&
	       left->dretpc == right->dretpc &&
	       left->dretpcactz == right->dretpcactz &&
	       !memcmp(left->lib_lo, right->lib_lo, sizeof(left->lib_lo)) &&
	       !memcmp(left->lib_hi, right->lib_hi, sizeof(left->lib_hi)) &&
	       !memcmp(left->jump_lo, right->jump_lo,
		       sizeof(left->jump_lo)) &&
	       !memcmp(left->jump_hi, right->jump_hi,
		       sizeof(left->jump_hi));
}

static int c5_expect_parent(const struct dasics_hw_state *parent,
			    const char *phase, unsigned int failure)
{
	struct dasics_hw_state actual;
	int ret;

	ret = dasics_hw_save(&actual);
	if (ret)
		return ret;
	if (!c5_hw_states_equal(parent, &actual)) {
		pr_err("C5: parent state mismatch after %s %u\n", phase,
		       failure);
		return -EIO;
	}
	return 0;
}

static int c5_seed_parent(struct dasics_hw_state *parent,
			  int *data_slot, int *jump_slot)
{
	unsigned long jump_lo = (unsigned long)c5_target_b;

	dasics_hw_clear_call_authority();
	register_kdasics((unsigned long)c5_target_b);
	*data_slot = dasics_libcfg_kalloc(DASICS_LIBCFG_R,
					  (unsigned long)c5_parent_data +
					  sizeof(c5_parent_data),
					  (unsigned long)c5_parent_data);
	if (*data_slot < 0)
		return -ENOSPC;
	*jump_slot = dasics_jumpcfg_kalloc(jump_lo, jump_lo + 4);
	if (*jump_slot < 0)
		return -ENOSPC;
	return dasics_hw_save(parent);
}

static int c5_test_success(const struct dasics_hw_state *parent)
{
	struct dasics_hw_state child;
	unsigned long expected_cfg;
	unsigned long expected_stack_hi;
	bool found[ARRAY_SIZE(c5_regions)] = { };
	unsigned int data_valid = 0;
	unsigned int jump_valid = 0;
	unsigned int i;
	int ret;

	memset(&c5_frame, 0, sizeof(c5_frame));
	asm volatile("mv %0, sp" : "=r" (expected_stack_hi));
	ret = dasics_call_prepare(&c5_frame, &c5_policy);
	if (ret)
		return ret;
	ret = dasics_hw_save(&child);
	if (ret)
		goto finish;

	for (i = 0; i < DASICS_DATA_BOUND_SLOTS; i++) {
		unsigned long cfg = (child.libcfg >>
				     (i * DASICS_LIBCFG_BITS)) &
				    DASICS_LIBCFG_MASK;

		if (cfg & DASICS_LIBCFG_V)
			data_valid++;
	}
	for (i = 0; i < DASICS_JUMP_BOUND_SLOTS; i++) {
		unsigned long cfg = (child.jumpcfg >>
				     (i * DASICS_JUMPCFG_BITS)) &
				    DASICS_JUMPCFG_MASK;

		if (cfg & DASICS_JUMPCFG_V)
			jump_valid++;
	}
	if (data_valid != ARRAY_SIZE(c5_regions) + 1 ||
	    jump_valid != ARRAY_SIZE(c5_code_ranges) ||
	    child.dmaincall != parent->dmaincall ||
	    c5_frame.nr_resident != ARRAY_SIZE(c5_regions) + 1 ||
	    c5_frame.nr_jump_bounds != ARRAY_SIZE(c5_code_ranges))
		ret = -EINVAL;
	expected_cfg = DASICS_LIBCFG_V | DASICS_LIBCFG_R |
		       DASICS_LIBCFG_W;
	if (((child.libcfg >> 0) & DASICS_LIBCFG_MASK) != expected_cfg ||
	    child.lib_hi[0] != expected_stack_hi ||
	    child.lib_hi[0] - child.lib_lo[0] != c5_policy.stack_size ||
	    child.jump_lo[0] != c5_code_ranges[0].base ||
	    child.jump_hi[0] != c5_code_ranges[0].base +
				c5_code_ranges[0].size ||
	    child.jump_lo[1] != c5_code_ranges[1].base ||
	    child.jump_hi[1] != c5_code_ranges[1].base +
				c5_code_ranges[1].size)
		ret = -EINVAL;
	for (i = 1; i < ARRAY_SIZE(c5_regions) + 1; i++) {
		unsigned long cfg = (child.libcfg >>
				     (i * DASICS_LIBCFG_BITS)) &
				    DASICS_LIBCFG_MASK;
		unsigned int region;

		for (region = 0; region < ARRAY_SIZE(c5_regions); region++) {
			unsigned long region_cfg = DASICS_LIBCFG_V;

			if (c5_regions[region].perms & DASICS_REGION_READ)
				region_cfg |= DASICS_LIBCFG_R;
			if (c5_regions[region].perms & DASICS_REGION_WRITE)
				region_cfg |= DASICS_LIBCFG_W;
			if (child.lib_lo[i] == c5_regions[region].base &&
			    child.lib_hi[i] == c5_regions[region].base +
					       c5_regions[region].size &&
			    cfg == region_cfg)
				found[region] = true;
		}
	}
	for (i = 0; i < ARRAY_SIZE(found); i++) {
		if (!found[i])
			ret = -EINVAL;
	}

finish:
	dasics_call_finish(&c5_frame);
	dasics_call_finish(&c5_frame);
	if (ret)
		return ret;
	ret = c5_expect_parent(parent, "successful finish", 0);
	if (!ret)
		pr_info("C5: PASS prepare installs child and finish restores parent\n");
	return ret;
}

static int c5_test_each_install_failure(const struct dasics_hw_state *parent)
{
	const unsigned int nr_installed = ARRAY_SIZE(c5_regions) + 1 +
					  ARRAY_SIZE(c5_code_ranges);
	unsigned int failure;
	int ret;

	for (failure = 1; failure <= nr_installed; failure++) {
		memset(&c5_frame, 0, sizeof(c5_frame));
		ret = dasics_call_test_fail_bound(&c5_frame, failure);
		if (ret)
			return ret;
		ret = dasics_call_prepare(&c5_frame, &c5_policy);
		if (ret != -EIO || c5_frame.prepare_error != -EIO ||
		    c5_frame.state != DASICS_CALL_FRAME_CLEANED)
			return -EINVAL;
		dasics_call_finish(&c5_frame);
		dasics_call_finish(&c5_frame);
		ret = c5_expect_parent(parent, "injected failure", failure);
		if (ret)
			return ret;
	}

	pr_info("C5: PASS all %u bound-install failures restore parent\n",
		nr_installed);
	return 0;
}

static int c5_test_partial_initialization(const struct dasics_hw_state *parent)
{
	struct dasics_call_policy invalid = c5_policy;
	int ret;

	memset(&c5_frame, 0, sizeof(c5_frame));
	invalid.stack_size = 0;
	ret = dasics_call_prepare(&c5_frame, &invalid);
	if (ret != -EINVAL || c5_frame.state != DASICS_CALL_FRAME_CLEANED)
		return -EINVAL;
	dasics_call_finish(&c5_frame);
	dasics_call_finish(NULL);
	ret = c5_expect_parent(parent, "partial initialization", 0);
	if (!ret)
		pr_info("C5: PASS partial initialization and repeated finish\n");
	return ret;
}

static int c5_test_restore_failure(void)
{
	struct dasics_hw_state cleared;
	int ret;

	memset(&c5_frame, 0, sizeof(c5_frame));
	ret = dasics_call_test_fail_restore(&c5_frame, true);
	if (ret)
		return ret;
	ret = dasics_call_prepare(&c5_frame, &c5_policy);
	if (ret)
		return ret;
	dasics_call_finish(&c5_frame);
	if (c5_frame.finish_error != -EIO || c5_frame.preempt_held)
		return -EINVAL;
	ret = dasics_hw_save(&cleared);
	if (ret)
		return ret;
	if (cleared.libcfg || cleared.jumpcfg || cleared.dmaincall ||
	    cleared.dretpc || cleared.dretpcactz)
		return -EIO;

	pr_info("C5: PASS restore failure clears authority and releases context\n");
	return 0;
}

static int __init c5_test_init(void)
{
	struct dasics_hw_state original;
	struct dasics_hw_state parent;
	int data_slot = -1;
	int jump_slot = -1;
	bool original_saved = false;
	int ret;

	ret = dasics_hw_save(&original);
	if (ret)
		goto out;
	original_saved = true;
	ret = c5_seed_parent(&parent, &data_slot, &jump_slot);
	if (ret)
		goto out;
	ret = c5_test_success(&parent);
	if (ret)
		goto out;
	ret = c5_test_each_install_failure(&parent);
	if (ret)
		goto out;
	ret = c5_test_partial_initialization(&parent);
	if (ret)
		goto out;
	ret = c5_test_restore_failure();

out:
	dasics_call_finish(&c5_frame);
	if (data_slot >= 0)
		dasics_libcfg_kfree(data_slot);
	if (jump_slot >= 0)
		dasics_jumpcfg_kfree(jump_slot);
	if (original_saved) {
		int restore_ret = dasics_hw_restore(&original);

		if (!ret)
			ret = restore_ret;
	}
	if (ret) {
		pr_err("C5: FAIL transaction tests: %d\n", ret);
		return ret;
	}

	pr_info("C5: PASS all call transaction tests\n");
	return 0;
}

static void __exit c5_test_exit(void)
{
	pr_info("C5: test module unloaded\n");
}

module_init(c5_test_init);
module_exit(c5_test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS C5 call transaction tests");
