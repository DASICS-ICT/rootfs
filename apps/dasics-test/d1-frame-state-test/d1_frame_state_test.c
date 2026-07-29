// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

static struct dasics_call_frame d1_frame;
static unsigned long d1_parent_data[2];
static unsigned long d1_child_data[2];

static noinline void d1_target(void)
{
}

static const struct dasics_code_range d1_code_ranges[] = {
	{ .base = (unsigned long)d1_target, .size = 4 },
};

static const struct dasics_compartment d1_compartment = {
	.code_ranges = d1_code_ranges,
	.nr_code_ranges = ARRAY_SIZE(d1_code_ranges),
};

static const struct dasics_region d1_regions[] = {
	{
		.base = (unsigned long)d1_child_data,
		.size = sizeof(d1_child_data),
		.perms = DASICS_REGION_READ | DASICS_REGION_WRITE,
	},
};

static const struct dasics_call_policy d1_policy = {
	.callee = (struct dasics_compartment *)&d1_compartment,
	.target = d1_target,
	.regions = d1_regions,
	.nr_regions = ARRAY_SIZE(d1_regions),
	.stack_size = 512,
};

static bool d1_hw_states_equal(const struct dasics_hw_state *left,
			       const struct dasics_hw_state *right)
{
	return left->libcfg == right->libcfg &&
	       left->jumpcfg == right->jumpcfg &&
	       left->dmaincall == right->dmaincall &&
	       left->dretpc == right->dretpc &&
	       left->dretpcactz == right->dretpcactz &&
	       !memcmp(left->lib_lo, right->lib_lo, sizeof(left->lib_lo)) &&
	       !memcmp(left->lib_hi, right->lib_hi, sizeof(left->lib_hi)) &&
	       !memcmp(left->jump_lo, right->jump_lo, sizeof(left->jump_lo)) &&
	       !memcmp(left->jump_hi, right->jump_hi, sizeof(left->jump_hi));
}

static int d1_expect_parent(const struct dasics_hw_state *parent,
			    const char *test)
{
	struct dasics_hw_state actual;
	int ret;

	ret = dasics_hw_save(&actual);
	if (ret)
		return ret;
	if (!d1_hw_states_equal(parent, &actual)) {
		pr_err("D1: parent state mismatch after %s\n", test);
		return -EIO;
	}
	return 0;
}

static int d1_prepare(void)
{
	memset(&d1_frame, 0, sizeof(d1_frame));
	return dasics_call_prepare(&d1_frame, &d1_policy);
}

static int d1_test_normal(const struct dasics_hw_state *parent)
{
	int ret;

	ret = d1_prepare();
	if (ret || d1_frame.state != DASICS_CALL_FRAME_PREPARED)
		return ret ?: -EINVAL;
	ret = dasics_call_test_transition(&d1_frame,
					  DASICS_CALL_FRAME_ENTERED);
	if (ret || d1_frame.state != DASICS_CALL_FRAME_ENTERED)
		return ret ?: -EINVAL;
	ret = dasics_call_test_transition(&d1_frame,
					  DASICS_CALL_FRAME_RETURNED);
	if (ret || d1_frame.state != DASICS_CALL_FRAME_RETURNED)
		return ret ?: -EINVAL;
	dasics_call_finish(&d1_frame);
	dasics_call_finish(&d1_frame);
	if (d1_frame.state != DASICS_CALL_FRAME_CLEANED)
		return -EINVAL;
	ret = d1_expect_parent(parent, "normal return");
	if (!ret)
		pr_info("D1: PASS normal return reaches shared cleanup\n");
	return ret;
}

static int d1_test_fault(const struct dasics_hw_state *parent)
{
	int ret;

	ret = d1_prepare();
	if (ret)
		return ret;
	ret = dasics_call_test_transition(&d1_frame,
					  DASICS_CALL_FRAME_ENTERED);
	if (ret)
		return ret;
	ret = dasics_call_recover(-EACCES);
	if (ret || d1_frame.state != DASICS_CALL_FRAME_FAULTED ||
	    d1_frame.fault_error != -EACCES)
		return ret ?: -EINVAL;
	dasics_call_finish(&d1_frame);
	if (d1_frame.state != DASICS_CALL_FRAME_CLEANED)
		return -EINVAL;
	ret = d1_expect_parent(parent, "fault recovery");
	if (!ret)
		pr_info("D1: PASS fault recovery reaches shared cleanup\n");
	return ret;
}

static int d1_test_illegal_transition(const struct dasics_hw_state *parent)
{
	int ret;

	ret = d1_prepare();
	if (ret)
		return ret;
	ret = dasics_call_test_transition(&d1_frame,
					  DASICS_CALL_FRAME_RETURNED);
	if (ret != -EPROTO || d1_frame.state != DASICS_CALL_FRAME_CLEANED ||
	    d1_frame.state_error != -EPROTO)
		return -EINVAL;
	dasics_call_finish(&d1_frame);
	ret = d1_expect_parent(parent, "illegal transition");
	if (!ret)
		pr_info("D1: PASS illegal transition fails closed\n");
	return ret;
}

static int d1_test_partial_prepare(const struct dasics_hw_state *parent)
{
	struct dasics_call_policy invalid = d1_policy;
	int ret;

	memset(&d1_frame, 0, sizeof(d1_frame));
	invalid.stack_size = 0;
	ret = dasics_call_prepare(&d1_frame, &invalid);
	if (ret != -EINVAL || d1_frame.prepare_error != -EINVAL ||
	    d1_frame.state != DASICS_CALL_FRAME_CLEANED)
		return -EINVAL;
	dasics_call_finish(&d1_frame);
	dasics_call_finish(&d1_frame);
	ret = d1_expect_parent(parent, "partial prepare");
	if (!ret)
		pr_info("D1: PASS partial prepare and repeated finish\n");
	return ret;
}

static int __init d1_test_init(void)
{
	struct dasics_hw_state original;
	struct dasics_hw_state parent;
	int data_slot = -1;
	bool original_saved = false;
	int ret;

	ret = dasics_hw_save(&original);
	if (ret)
		goto out;
	original_saved = true;
	dasics_hw_clear_call_authority();
	register_kdasics((unsigned long)d1_target);
	data_slot = dasics_libcfg_kalloc(DASICS_LIBCFG_R,
					 (unsigned long)d1_parent_data +
					 sizeof(d1_parent_data),
					 (unsigned long)d1_parent_data);
	if (data_slot < 0) {
		ret = -ENOSPC;
		goto out;
	}
	ret = dasics_hw_save(&parent);
	if (ret)
		goto out;
	ret = d1_test_normal(&parent);
	if (ret)
		goto out;
	ret = d1_test_fault(&parent);
	if (ret)
		goto out;
	ret = d1_test_illegal_transition(&parent);
	if (ret)
		goto out;
	ret = d1_test_partial_prepare(&parent);

out:
	dasics_call_finish(&d1_frame);
	if (data_slot >= 0)
		dasics_libcfg_kfree(data_slot);
	if (original_saved) {
		int restore_ret = dasics_hw_restore(&original);

		if (!ret)
			ret = restore_ret;
	}
	if (ret) {
		pr_err("D1: FAIL frame state tests: %d\n", ret);
		return ret;
	}

	pr_info("D1: PASS all frame state tests\n");
	return 0;
}

static void __exit d1_test_exit(void)
{
	pr_info("D1: test module unloaded\n");
}

module_init(d1_test_init);
module_exit(d1_test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS D1 call-frame state machine tests");
