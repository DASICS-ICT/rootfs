// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../e1-utmod/e1_utmod.h"

static struct dasics_call_frame e1_frame;
static struct dasics_compartment e1_compartment;
static u8 e1_stack_probe[1024] __aligned(16);

static long e1_run(void *target, unsigned long arg1,
		   struct dasics_call_regs *regs)
{
	struct dasics_call_policy policy = {
		.callee = &e1_compartment,
		.target = target,
		.stack_size = 512,
	};

	memset(&e1_frame, 0, sizeof(e1_frame));
	memset(regs, 0, sizeof(*regs));
	regs->a0 = (unsigned long)dasics_maincall_gate;
	regs->a1 = arg1;
	return dasics_call(&e1_frame, &policy, regs);
}

static int __init e1_tmod_init(void)
{
	struct dasics_call_regs regs;
	unsigned int i;
	long ret;

	ret = dasics_compartment_init_module(&e1_compartment,
					     e1_valid_service);
	if (ret)
		goto fail;

	ret = e1_run(e1_valid_service, 0, &regs);
	if (ret || regs.ret_a0 ||
	    regs.ret_a1 != DASICS_MAINCALL_ABI_VERSION)
		goto fail;
	pr_info("E1: PASS legal read-only service returned ABI version %lu\n",
		regs.ret_a1);

	ret = e1_run(e1_full_registers, 0, &regs);
	if (ret || regs.ret_a0 ||
	    regs.ret_a1 != DASICS_MAINCALL_REGISTER_TEST_VALUE)
		goto fail;
	pr_info("E1: PASS a0-a7 request registers preserved\n");

	ret = e1_run(e1_unknown_service, 0, &regs);
	if (ret || (long)regs.ret_a0 != -ENOSYS || regs.ret_a1)
		goto fail;
	pr_info("E1: PASS unknown service ID rejected\n");

	ret = e1_run(e1_bad_arguments, 0, &regs);
	if (ret || (long)regs.ret_a0 != -EINVAL || regs.ret_a1)
		goto fail;
	pr_info("E1: PASS malformed service arguments rejected\n");

	ret = e1_run(e1_invalid_source, 0, &regs);
	if (ret != -EPERM || e1_frame.state != DASICS_CALL_FRAME_CLEANED ||
	    e1_frame.parent_saved || e1_frame.preempt_held)
		goto fail;
	pr_info("E1: PASS invalid source unwound to outer continuation\n");

	memset(e1_stack_probe, 0x5a, sizeof(e1_stack_probe));
	ret = e1_run(e1_untrusted_stack,
		     (unsigned long)&e1_stack_probe[768], &regs);
	if (ret || regs.ret_a0 ||
	    regs.ret_a1 != DASICS_MAINCALL_ABI_VERSION)
		goto fail;
	for (i = 0; i < ARRAY_SIZE(e1_stack_probe); i++) {
		if (e1_stack_probe[i] != 0x5a)
			goto fail;
	}
	pr_info("E1: PASS maincall gate ignored untrusted-supplied stack\n");
	pr_info("E1: PASS fixed maincall ABI and whitelist dispatch tests\n");
	return 0;

fail:
	dasics_call_finish(&e1_frame);
	dasics_compartment_destroy(&e1_compartment);
	pr_err("E1: FAIL ret=%ld service_status=%ld value=%lu state=%u saved=%u preempt=%u\n",
	       ret, (long)regs.ret_a0, regs.ret_a1, e1_frame.state,
	       e1_frame.parent_saved, e1_frame.preempt_held);
	return ret ?: -EINVAL;
}

static void __exit e1_tmod_exit(void)
{
	dasics_compartment_destroy(&e1_compartment);
}

module_init(e1_tmod_init);
module_exit(e1_tmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS E1 maincall ABI and whitelist tests");
