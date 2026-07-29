// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../e5-utmod/e5_utmod.h"

static struct dasics_call_frame e5_root_frame;
static struct dasics_call_frame e5_child_frame;
static struct dasics_call_frame e5_depth_frames[DASICS_CALL_MAX_DEPTH];
static struct dasics_compartment e5_compartment;
static unsigned long e5_fault_word = 0xE5E5E5E5UL;

static long e5_nested_service(const struct dasics_maincall_request *request,
			      unsigned long *value)
{
	struct dasics_call_regs regs;
	struct dasics_call_policy policy = {
		.callee = &e5_compartment,
		.stack_size = 512,
	};
	struct dasics_region fault_region = {
		.base = (unsigned long)&e5_fault_word,
		.size = sizeof(e5_fault_word),
		.perms = DASICS_REGION_READ,
	};
	long ret;
	unsigned int i;

	for (i = request->args[0] == E5_MODE_DEPTH ? 2 : 1;
	     i < ARRAY_SIZE(request->args); i++) {
		if (request->args[i])
			return -EINVAL;
	}

	memset(&regs, 0, sizeof(regs));
	switch (request->args[0]) {
	case E5_MODE_NORMAL:
		memset(&e5_child_frame, 0, sizeof(e5_child_frame));
		policy.target = e5_child_normal;
		regs.a0 = (unsigned long)dasics_maincall_gate;
		ret = dasics_call(&e5_child_frame, &policy, &regs);
		if (ret)
			return ret;
		if (regs.ret_a0 != E5_CHILD_MAGIC)
			return -EPROTO;
		*value = regs.ret_a0;
		return 0;
	case E5_MODE_FAULT:
		memset(&e5_child_frame, 0, sizeof(e5_child_frame));
		policy.target = e5_child_fault;
		policy.regions = &fault_region;
		policy.nr_regions = 1;
		regs.a0 = (unsigned long)&e5_fault_word;
		ret = dasics_call(&e5_child_frame, &policy, &regs);
		if (ret != -EFAULT)
			return ret ?: -EPROTO;
		if (e5_fault_word != 0xE5E5E5E5UL)
			return -EUCLEAN;
		return -EFAULT;
	case E5_MODE_DEPTH: {
		struct dasics_call_frame *parent_frame =
			dasics_call_current_frame();
		struct dasics_call_frame *child;

		if (!parent_frame ||
		    parent_frame->depth >= DASICS_CALL_MAX_DEPTH ||
		    request->args[1] > DASICS_CALL_MAX_DEPTH - 2)
			return -EINVAL;
		child = &e5_depth_frames[parent_frame->depth];
		memset(child, 0, sizeof(*child));
		policy.target = e5_child_depth;
		regs.a0 = (unsigned long)dasics_maincall_gate;
		regs.a1 = request->args[1];
		ret = dasics_call(child, &policy, &regs);
		if (ret)
			return ret;
		if (regs.ret_a0 != E5_DEPTH_MAGIC)
			return -EPROTO;
		*value = regs.ret_a0;
		return 0;
	}
	default:
		return -EINVAL;
	}
}

static long e5_run_root(void *target)
{
	struct dasics_call_regs regs;
	struct dasics_call_policy policy = {
		.callee = &e5_compartment,
		.target = target,
		.stack_size = 1024,
	};
	long ret;

	memset(&e5_root_frame, 0, sizeof(e5_root_frame));
	memset(&regs, 0, sizeof(regs));
	regs.a0 = (unsigned long)dasics_maincall_gate;
	ret = dasics_call(&e5_root_frame, &policy, &regs);
	if (ret)
		return ret;
	return regs.ret_a0;
}

static int __init e5_tmod_init(void)
{
	struct dasics_hw_state before;
	struct dasics_hw_state after;
	long ret;

	ret = dasics_compartment_init_module(&e5_compartment,
					     e5_outer_normal);
	if (ret)
		goto fail;
	ret = dasics_maincall_debug_register(e5_nested_service);
	if (ret)
		goto fail;
	ret = dasics_hw_save(&before);
	if (ret)
		goto fail_unregister;

	ret = e5_run_root(e5_outer_normal);
	if (ret != E5_OUTER_NORMAL_MAGIC)
		goto fail_unregister;
	if (e5_child_frame.parent != &e5_root_frame ||
	    e5_child_frame.depth != 1 ||
	    e5_child_frame.state != DASICS_CALL_FRAME_CLEANED)
		goto fail_unregister;
	pr_info("E5: PASS nested child returned and outer continued\n");

	ret = e5_run_root(e5_outer_fault);
	if (ret != E5_OUTER_FAULT_MAGIC ||
	    e5_fault_word != 0xE5E5E5E5UL ||
	    !e5_child_frame.fault.valid ||
	    e5_child_frame.fault.reason != DASICS_FAULT_STORE)
		goto fail_unregister;
	pr_info("E5: PASS nested child fault restored outer continuation\n");

	memset(e5_depth_frames, 0, sizeof(e5_depth_frames));
	ret = e5_run_root(e5_outer_depth);
	if (ret != E5_OUTER_DEPTH_MAGIC ||
	    e5_depth_frames[0].parent != &e5_root_frame ||
	    e5_depth_frames[0].depth != 1 ||
	    e5_depth_frames[1].parent != &e5_depth_frames[0] ||
	    e5_depth_frames[1].depth != 2 ||
	    e5_depth_frames[2].parent != &e5_depth_frames[1] ||
	    e5_depth_frames[2].depth != 3 ||
	    e5_depth_frames[3].prepare_error != -EOVERFLOW)
		goto fail_unregister;
	pr_info("E5: PASS nested depth overflow failed closed\n");

	ret = dasics_hw_save(&after);
	if (ret || memcmp(&before, &after, sizeof(before)))
		goto fail_unregister;
	if (dasics_call_current_frame())
		goto fail_unregister;
	pr_info("E5: PASS root cleanup restored hardware and empty frame stack\n");

	dasics_compartment_destroy(&e5_compartment);
	ret = e5_run_root(e5_outer_normal);
	if (ret != -ENODEV || dasics_call_current_frame())
		goto fail_unregister;
	pr_info("E5: PASS destroyed module compartment rejected stale call\n");
	pr_info("E5: PASS single-hart nested frame tests\n");
	dasics_maincall_debug_unregister(e5_nested_service);
	return 0;

fail_unregister:
	dasics_call_finish(&e5_child_frame);
	dasics_call_finish(&e5_root_frame);
	dasics_compartment_destroy(&e5_compartment);
	dasics_maincall_debug_unregister(e5_nested_service);
fail:
	pr_err("E5: FAIL ret=%ld root_state=%u child_state=%u child_depth=%u fault=%lu\n",
	       ret, e5_root_frame.state, e5_child_frame.state,
	       e5_child_frame.depth, e5_child_frame.fault.reason);
	return ret < 0 ? ret : -EINVAL;
}

static void __exit e5_tmod_exit(void)
{
	dasics_compartment_destroy(&e5_compartment);
	dasics_maincall_debug_unregister(e5_nested_service);
}

module_init(e5_tmod_init);
module_exit(e5_tmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS E5 single-hart nested frame tests");
