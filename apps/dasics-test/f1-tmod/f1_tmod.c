// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../f1-utmod/f1_utmod.h"

static struct dasics_call_frame f1_frame;
static struct dasics_compartment f1_compartment;
static unsigned long f1_forbidden = 0xf1;

static bool f1_hw_states_equal(const struct dasics_hw_state *left,
			       const struct dasics_hw_state *right)
{
	return !memcmp(left, right, sizeof(*left));
}

static long f1_run(void *target, unsigned long arg0,
		   struct dasics_call_regs *regs)
{
	struct dasics_call_policy policy = {
		.callee = &f1_compartment,
		.target = target,
		.stack_size = 512,
	};

	memset(&f1_frame, 0, sizeof(f1_frame));
	memset(regs, 0, sizeof(*regs));
	regs->a0 = arg0;
	return dasics_call(&f1_frame, &policy, regs);
}

static int __init f1_tmod_init(void)
{
	struct dasics_hw_state before;
	struct dasics_hw_state after;
	struct dasics_call_regs regs = {};
	long ret;

	ret = dasics_compartment_init_module(&f1_compartment,
					     f1_sleep_then_abi);
	if (ret)
		goto fail;
	ret = dasics_hw_save(&before);
	if (ret)
		goto fail;

	ret = f1_run(f1_sleep_then_abi,
		     (unsigned long)dasics_maincall_gate, &regs);
	if (ret || regs.ret_a0 ||
	    regs.ret_a1 != DASICS_MAINCALL_SLEEP_TEST_VALUE ||
	    f1_frame.state != DASICS_CALL_FRAME_CLEANED ||
	    f1_frame.maincall_suspended || f1_frame.preempt_held)
		goto fail;
	pr_info("F1: PASS sleepable maincall blocked, woke, and re-entered maincall\n");

	ret = f1_run(f1_clobber_return_context, 1, &regs);
	if (ret || regs.ret_a0 != 0x51 || regs.ret_a1 != 0x52)
		goto fail;
	pr_info("F1: PASS normal return ignored untrusted sp/gp/tp/s1\n");

	ret = f1_run(f1_clobber_then_fault,
		     (unsigned long)&f1_forbidden, &regs);
	if (ret != -EFAULT || !f1_frame.fault.valid ||
	    f1_frame.fault.reason != DASICS_FAULT_LOAD ||
	    f1_frame.state != DASICS_CALL_FRAME_CLEANED)
		goto fail;
	pr_info("F1: PASS fault entry ignored untrusted sp/gp/tp/s1\n");

	ret = dasics_hw_save(&after);
	if (ret || !f1_hw_states_equal(&before, &after))
		goto fail;
	pr_info("F1: PASS parent CSR state restored after sleep and return\n");
	return 0;

fail:
	dasics_call_finish(&f1_frame);
	dasics_compartment_destroy(&f1_compartment);
	pr_err("F1: FAIL ret=%ld service_status=%ld value=%lx state=%u suspended=%u preempt=%u\n",
	       ret, (long)regs.ret_a0, regs.ret_a1, f1_frame.state,
	       f1_frame.maincall_suspended, f1_frame.preempt_held);
	return ret ?: -EINVAL;
}

static void __exit f1_tmod_exit(void)
{
	dasics_compartment_destroy(&f1_compartment);
}

module_init(f1_tmod_init);
module_exit(f1_tmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS F1 sleepable-maincall regression");
