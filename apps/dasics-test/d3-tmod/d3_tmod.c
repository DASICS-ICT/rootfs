// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../d3-utmod/d3_utmod.h"

static struct dasics_call_frame d3_frame;
static struct dasics_compartment d3_compartment;
static struct d3_cell d3_cells[D3_NR_CELLS];
static struct dasics_region d3_regions[D3_NR_CELLS];

static bool d3_hw_states_equal(const struct dasics_hw_state *left,
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

static int d3_expect_parent(const struct dasics_hw_state *parent)
{
	struct dasics_hw_state actual;
	int ret;

	ret = dasics_hw_save(&actual);
	if (ret)
		return ret;
	return d3_hw_states_equal(parent, &actual) ? 0 : -EIO;
}

static void d3_init_regions(unsigned int last_perms)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(d3_regions); i++) {
		d3_regions[i].base = (unsigned long)&d3_cells[i].value;
		d3_regions[i].size = sizeof(d3_cells[i].value);
		d3_regions[i].perms = i == ARRAY_SIZE(d3_regions) - 1 ?
			last_perms : DASICS_REGION_READ | DASICS_REGION_WRITE;
	}
}

static int d3_test_refill(const struct dasics_hw_state *parent)
{
	struct dasics_call_regs regs = { };
	struct dasics_call_policy policy = {
		.callee = &d3_compartment,
		.target = d3_touch_cells,
		.regions = d3_regions,
		.nr_regions = ARRAY_SIZE(d3_regions),
		.stack_size = 512,
	};
	unsigned int i;
	long ret;

	memset(d3_cells, 0, sizeof(d3_cells));
	d3_init_regions(DASICS_REGION_READ | DASICS_REGION_WRITE);
	memset(&d3_frame, 0, sizeof(d3_frame));
	regs.a0 = (unsigned long)d3_cells;
	regs.a1 = ARRAY_SIZE(d3_cells);
	ret = dasics_call(&d3_frame, &policy, &regs);
	if (ret || regs.ret_a0 != 210 || d3_frame.fault.valid ||
	    d3_frame.data_refills < 6 ||
	    d3_frame.data_misses != d3_frame.data_refills)
		return -EINVAL;
	for (i = 0; i < ARRAY_SIZE(d3_cells); i++) {
		if (d3_cells[i].value != i + 1)
			return -EIO;
	}
	ret = d3_expect_parent(parent);
	if (!ret)
		pr_info("D3: PASS %zu regions refilled %u misses and retried\n",
			ARRAY_SIZE(d3_regions), d3_frame.data_refills);
	return ret;
}

static int d3_test_permission_mismatch(const struct dasics_hw_state *parent)
{
	struct dasics_call_regs regs = { };
	struct dasics_call_policy policy = {
		.callee = &d3_compartment,
		.target = d3_write_one,
		.regions = &d3_regions[D3_NR_CELLS - 1],
		.nr_regions = 1,
		.stack_size = 512,
	};
	u64 original = d3_cells[D3_NR_CELLS - 1].value;
	long ret;

	d3_init_regions(DASICS_REGION_READ);
	memset(&d3_frame, 0, sizeof(d3_frame));
	regs.a0 = (unsigned long)&d3_cells[D3_NR_CELLS - 1].value;
	ret = dasics_call(&d3_frame, &policy, &regs);
	if (ret != -EFAULT || !d3_frame.fault.valid ||
	    d3_frame.fault.reason != DASICS_FAULT_STORE ||
	    d3_cells[D3_NR_CELLS - 1].value != original)
		return -EINVAL;
	ret = d3_expect_parent(parent);
	if (!ret)
		pr_info("D3: PASS write permission mismatch rejected\n");
	return ret;
}

static int d3_test_pinned_exhaustion(const struct dasics_hw_state *parent)
{
	struct dasics_call_policy policy = {
		.callee = &d3_compartment,
		.target = d3_touch_cells,
		.regions = d3_regions,
		.nr_regions = ARRAY_SIZE(d3_regions),
		.stack_size = 512,
	};
	unsigned long address = d3_regions[D3_NR_CELLS - 1].base;
	unsigned int i;
	int ret;

	d3_init_regions(DASICS_REGION_READ | DASICS_REGION_WRITE);
	memset(&d3_frame, 0, sizeof(d3_frame));
	ret = dasics_call_prepare(&d3_frame, &policy);
	if (ret)
		return ret;
	for (i = 0; i < ARRAY_SIZE(d3_frame.data_bounds.entries); i++) {
		struct dasics_bound_entry *entry = &d3_frame.data_bounds.entries[i];

		if (entry->active && entry->resident_slot >= 0)
			entry->flags |= DASICS_BOUND_PINNED;
	}
	ret = dasics_call_test_transition(&d3_frame,
					  DASICS_CALL_FRAME_ENTERED);
	if (ret)
		goto finish;
	ret = dasics_call_test_handle_trap((unsigned long)d3_touch_cells,
					   address, DASICS_FAULT_STORE,
					   EXC_DASICS_SCHECK_FAULT);
	if (ret != DASICS_TRAP_TERMINAL || !d3_frame.fault.valid ||
	    d3_frame.data_refills)
		ret = -EINVAL;
	else
		ret = 0;

finish:
	dasics_call_finish(&d3_frame);
	if (!ret)
		ret = d3_expect_parent(parent);
	if (!ret)
		pr_info("D3: PASS pinned slot exhaustion failed closed\n");
	return ret;
}

static int d3_test_unknown_address(const struct dasics_hw_state *parent)
{
	struct dasics_call_policy policy = {
		.callee = &d3_compartment,
		.target = d3_touch_cells,
		.regions = d3_regions,
		.nr_regions = ARRAY_SIZE(d3_regions),
		.stack_size = 512,
	};
	unsigned long address;
	int ret;

	d3_init_regions(DASICS_REGION_READ | DASICS_REGION_WRITE);
	address = d3_regions[D3_NR_CELLS - 1].base +
		 2 * sizeof(struct d3_cell);
	memset(&d3_frame, 0, sizeof(d3_frame));
	ret = dasics_call_prepare(&d3_frame, &policy);
	if (ret)
		return ret;
	ret = dasics_call_test_transition(&d3_frame,
					  DASICS_CALL_FRAME_ENTERED);
	if (ret)
		goto finish;
	ret = dasics_call_test_handle_trap((unsigned long)d3_touch_cells,
					   address, DASICS_FAULT_LOAD,
					   EXC_DASICS_SCHECK_FAULT);
	if (ret != DASICS_TRAP_TERMINAL || !d3_frame.fault.valid ||
	    d3_frame.fault.address != address || d3_frame.data_refills)
		ret = -EINVAL;
	else
		ret = 0;

finish:
	dasics_call_finish(&d3_frame);
	if (!ret)
		ret = d3_expect_parent(parent);
	if (!ret)
		pr_info("D3: PASS unknown address failed closed\n");
	return ret;
}

static int __init d3_tmod_init(void)
{
	struct dasics_hw_state original;
	struct dasics_hw_state parent;
	bool original_saved = false;
	int ret;

	ret = dasics_hw_save(&original);
	if (ret)
		goto out;
	original_saved = true;
	ret = dasics_compartment_init_module(&d3_compartment, d3_touch_cells);
	if (ret)
		goto out;
	ret = dasics_hw_save(&parent);
	if (ret)
		goto out;
	ret = d3_test_refill(&parent);
	if (ret)
		goto out;
	ret = d3_test_permission_mismatch(&parent);
	if (ret)
		goto out;
	ret = d3_test_pinned_exhaustion(&parent);
	if (ret)
		goto out;
	ret = d3_test_unknown_address(&parent);

out:
	dasics_call_finish(&d3_frame);
	dasics_compartment_destroy(&d3_compartment);
	if (original_saved) {
		int restore_ret = dasics_hw_restore(&original);

		if (!ret)
			ret = restore_ret;
	}
	if (ret) {
		pr_err("D3: FAIL data-bound refill tests: %d\n", ret);
		return ret;
	}
	pr_info("D3: PASS all data-bound refill tests\n");
	return 0;
}

static void __exit d3_tmod_exit(void)
{
	dasics_compartment_destroy(&d3_compartment);
}

module_init(d3_tmod_init);
module_exit(d3_tmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS D3 data-bound refill tests");
