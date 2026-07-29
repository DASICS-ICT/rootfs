// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../d4-utmod/d4_utmod.h"

static struct dasics_call_frame d4_frame;
static struct dasics_compartment d4_compartment;
static u64 d4_value = 0x12345678;
static u64 d4_after;

static bool d4_hw_states_equal(const struct dasics_hw_state *left,
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

static int d4_expect_parent(const struct dasics_hw_state *parent)
{
	struct dasics_hw_state actual;
	int ret;

	ret = dasics_hw_save(&actual);
	if (ret)
		return ret;
	return d4_hw_states_equal(parent, &actual) ? 0 : -EIO;
}

static int d4_run_fault(void *target, unsigned long a0,
			unsigned long a1, unsigned long reason,
			long expected_error,
			const struct dasics_hw_state *parent)
{
	struct dasics_call_regs regs = { .a0 = a0, .a1 = a1 };
	struct dasics_region after_region = {
		.base = (unsigned long)&d4_after,
		.size = sizeof(d4_after),
		.perms = DASICS_REGION_WRITE,
	};
	struct dasics_call_policy policy = {
		.callee = &d4_compartment,
		.target = target,
		.regions = &after_region,
		.nr_regions = 1,
		.stack_size = 512,
	};
	long ret;

	d4_after = 0;
	memset(&d4_frame, 0, sizeof(d4_frame));
	ret = dasics_call(&d4_frame, &policy, &regs);
	if (ret != expected_error || d4_after || !d4_frame.fault.valid ||
	    d4_frame.fault.reason != reason ||
	    d4_frame.state != DASICS_CALL_FRAME_CLEANED ||
	    d4_frame.parent_saved || d4_frame.preempt_held)
		goto fail;
	return d4_expect_parent(parent);

fail:
	pr_err("D4: unexpected result ret=%ld after=%llu valid=%u reason=%lu state=%u saved=%u preempt=%u\n",
	       ret, d4_after, d4_frame.fault.valid, d4_frame.fault.reason,
	       d4_frame.state, d4_frame.parent_saved, d4_frame.preempt_held);
	return -EINVAL;
}

static void noinline d4_forbidden_target(void)
{
	d4_value = 0xbad;
}

static int __init d4_tmod_init(void)
{
	struct dasics_hw_state parent;
	u64 original = d4_value;
	int ret;

	ret = dasics_compartment_init_module(&d4_compartment, d4_fault_load);
	if (ret)
		goto out;
	ret = dasics_hw_save(&parent);
	if (ret)
		goto out;

	ret = d4_run_fault(d4_fault_load, (unsigned long)&d4_value,
			   (unsigned long)&d4_after, DASICS_FAULT_LOAD,
			   -EFAULT, &parent);
	if (ret)
		goto out;
	pr_info("D4: PASS out-of-bounds read unwound\n");

	ret = d4_run_fault(d4_fault_store, (unsigned long)&d4_value,
			   (unsigned long)&d4_after, DASICS_FAULT_STORE,
			   -EFAULT, &parent);
	if (ret || d4_value != original)
		goto out;
	pr_info("D4: PASS out-of-bounds write unwound\n");

	ret = d4_run_fault(d4_fault_jump,
			   (unsigned long)d4_forbidden_target,
			   (unsigned long)&d4_after, DASICS_FAULT_JUMP,
			   -EPERM, &parent);
	if (ret || d4_value != original)
		goto out;
	pr_info("D4: PASS illegal jump unwound\n");

	ret = d4_run_fault(d4_fault_stack_low,
			   (unsigned long)&d4_after, 0,
			   DASICS_FAULT_STORE, -EFAULT, &parent);
	if (ret)
		goto out;
	pr_info("D4: PASS stack lower bound rejected underflow\n");

	ret = d4_run_fault(d4_fault_stack_high,
			   (unsigned long)&d4_after, 0,
			   DASICS_FAULT_STORE, -EFAULT, &parent);
	if (ret)
		goto out;
	pr_info("D4: PASS stack upper bound protected trusted caller frame\n");

out:
	dasics_call_finish(&d4_frame);
	dasics_compartment_destroy(&d4_compartment);
	if (ret) {
		pr_err("D4: FAIL fault recovery tests: %d\n", ret);
		return ret;
	}
	pr_info("D4: PASS all fault recovery tests\n");
	return 0;
}

static void __exit d4_tmod_exit(void)
{
	dasics_compartment_destroy(&d4_compartment);
}

module_init(d4_tmod_init);
module_exit(d4_tmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS D4 fault recovery tests");
