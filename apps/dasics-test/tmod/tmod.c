// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/kdasics.h>

#include "../utmod/utmod.h"

MODULE_LICENSE("GPL");

static bool mid_call_snapshot_taken;
static struct dasics_call_frame c6_frame;
static struct dasics_compartment c6_compartment;
static u64 c6_array[] = { 1, 2, 3 };
static struct dasics_region c6_regions[] = {
	{
		.base = (unsigned long)c6_array,
		.size = 2 * sizeof(c6_array[0]),
		.perms = DASICS_REGION_READ | DASICS_REGION_WRITE,
	},
};

static struct dasics_call_policy c6_policy = {
	.callee = &c6_compartment,
	.target = test_bound,
	.regions = c6_regions,
	.nr_regions = ARRAY_SIZE(c6_regions),
	/* Explicit trusted test budget, not a KSplit-derived stack extent. */
	.stack_size = 512,
};

static int capture_hw_state(const char *phase)
{
	struct dasics_hw_state state;
	int ret;

	ret = dasics_hw_save(&state);
	if (ret) {
		pr_err("C6: %s snapshot failed: %d\n", phase, ret);
		return ret;
	}

	pr_info("C6: %s authority libcfg=%lx jumpcfg=%lx dmaincall=%lx\n",
		phase, state.libcfg, state.jumpcfg, state.dmaincall);
	return 0;
}

static bool hw_states_equal(const struct dasics_hw_state *left,
			    const struct dasics_hw_state *right)
{
	unsigned int idx;

	if (left->libcfg != right->libcfg ||
	    left->jumpcfg != right->jumpcfg ||
	    left->dmaincall != right->dmaincall ||
	    left->dretpc != right->dretpc ||
	    left->dretpcactz != right->dretpcactz)
		return false;

	for (idx = 0; idx < DASICS_MAX_DATA_BOUNDS; idx++) {
		if (left->lib_lo[idx] != right->lib_lo[idx] ||
		    left->lib_hi[idx] != right->lib_hi[idx])
			return false;
	}

	for (idx = 0; idx < DASICS_MAX_JUMP_BOUNDS; idx++) {
		if (left->jump_lo[idx] != right->jump_lo[idx] ||
		    left->jump_hi[idx] != right->jump_hi[idx])
			return false;
	}

	return true;
}

static bool address_in_compartment(unsigned long address)
{
	unsigned int i;

	for (i = 0; i < c6_compartment.nr_code_ranges; i++) {
		const struct dasics_code_range *range =
			&c6_compartment.code_ranges[i];

		if (address >= range->base && address < range->base + range->size)
			return true;
	}
	return false;
}

static u64 tmod_smaincall(unsigned long type, u64 arg0, u64 arg1)
{
	if (!mid_call_snapshot_taken) {
		capture_hw_state("mid-call");
		mid_call_snapshot_taken = true;
	}

	if (type == 1)
		pr_info("%s", (const char *)arg0);
	else
		pr_warn("invalid smaincall number %lu\n", type);

	return 0;
}

static int init_tmod(void)
{
	struct dasics_call_policy invalid_policy;
	struct dasics_call_regs regs = { };
	struct dasics_hw_state original;
	struct dasics_hw_state parent;
	struct dasics_hw_state failed;
	struct dasics_hw_state after;
	unsigned int i;
	long ret;

	ret = dasics_hw_save(&original);
	if (ret)
		return ret;
	ret = dasics_compartment_init_module(&c6_compartment, test_bound);
	if (ret)
		return ret;
	for (i = 0; i < c6_compartment.nr_code_ranges; i++)
		pr_info("C6: loader code range[%u]=[%lx,%lx)\n", i,
			c6_compartment.code_ranges[i].base,
			c6_compartment.code_ranges[i].base +
			c6_compartment.code_ranges[i].size);

	register_kdasics((u64)tmod_smaincall);
	ret = dasics_hw_save(&parent);
	if (ret)
		goto restore;
	invalid_policy = c6_policy;
	invalid_policy.target = (void *)1;
	memset(&c6_frame, 0, sizeof(c6_frame));
	ret = dasics_call(&c6_frame, &invalid_policy, &regs);
	if (!ret)
		ret = -EINVAL;
	if (dasics_hw_save(&failed)) {
		ret = -EIO;
		goto restore;
	}
	if (ret != -EPERM || c6_array[0] != 1 || c6_array[1] != 2 ||
	    c6_array[2] != 3 || !hw_states_equal(&parent, &failed)) {
		ret = -EINVAL;
		goto restore;
	}
	pr_info("C6: PASS prepare failure did not enter utmod\n");

	memset(&c6_frame, 0, sizeof(c6_frame));
	memset(&regs, 0, sizeof(regs));
	regs.a0 = (unsigned long)c6_array;
	mid_call_snapshot_taken = false;
	pr_info("C6: two-element RW grant=[%lx,%lx), array[2] remains negative test\n",
		(unsigned long)c6_array, (unsigned long)(c6_array + 2));
	ret = dasics_call(&c6_frame, &c6_policy, &regs);
	if (ret != -EFAULT || regs.ret_a0 || c6_array[0] != 3 ||
	    c6_array[1] != 4 ||
	    c6_array[2] != 3) {
		pr_err("C6: call result=%ld ret_a0=%lx array={%llu,%llu,%llu}\n",
		       ret, regs.ret_a0, c6_array[0], c6_array[1], c6_array[2]);
		ret = -EIO;
		goto restore;
	}
	if (!c6_frame.fault.valid ||
	    c6_frame.fault.address != (unsigned long)(c6_array + 2) ||
	    c6_frame.fault.reason != DASICS_FAULT_STORE ||
	    c6_frame.fault.cause != EXC_DASICS_SCHECK_FAULT ||
	    c6_frame.fault.compartment != &c6_compartment ||
	    !address_in_compartment(c6_frame.fault.pc)) {
		pr_err("D2: invalid fault record valid=%u pc=%lx address=%lx reason=%lu cause=%lu compartment=%px\n",
		       c6_frame.fault.valid, c6_frame.fault.pc,
		       c6_frame.fault.address, c6_frame.fault.reason,
		       c6_frame.fault.cause, c6_frame.fault.compartment);
		ret = -EINVAL;
		goto restore;
	}
	pr_info("D2: PASS fault frame pc=%lx address=%lx reason=%lu cause=%lu compartment=utmod\n",
		c6_frame.fault.pc, c6_frame.fault.address,
		c6_frame.fault.reason, c6_frame.fault.cause);
	pr_info("C6: PASS legal writes succeeded and offset-16 write was rejected\n");

	ret = dasics_hw_save(&after);
	if (ret)
		goto restore;
	if (!hw_states_equal(&parent, &after)) {
		ret = -EIO;
		goto restore;
	}
	pr_info("C6: PASS normal return revoked temporary bounds and restored parent\n");
	pr_info("C6: PASS single-level dasics_call test\n");
	ret = 0;

restore:
	if (dasics_hw_restore(&original) && !ret)
		ret = -EIO;
	dasics_compartment_destroy(&c6_compartment);
	return ret;
}

static void exit_tmod(void)
{
	dasics_compartment_destroy(&c6_compartment);
	pr_info("exit tmod\n");
}

module_init(init_tmod);
module_exit(exit_tmod);
