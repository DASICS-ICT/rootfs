/* SPDX-License-Identifier: GPL-2.0-only */

#include "e5_utmod.h"

	.text
	.balign 4

	.macro e5_save_frame
	addi	sp, sp, -32
	sd	s0, 16(sp)
	sd	ra, 24(sp)
	.endm

	.macro e5_restore_frame
	ld	s0, 16(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 32
	ret
	.endm

	.macro e5_clear_request_tail
	li	a2, 0
	li	a3, 0
	li	a4, 0
	li	a5, 0
	li	a6, 0
	li	a7, 0
	.endm

	.macro e5_call_abi_info
	li	a0, E5_MAINCALL_SERVICE_ABI_INFO
	li	a1, E5_MAINCALL_QUERY_RUNTIME
	e5_clear_request_tail
	jalr	ra, s0, 0
	.endm

	.globl e5_outer_normal
	.type e5_outer_normal, @function
e5_outer_normal:
	e5_save_frame
	mv	s0, a0
	li	a0, E5_MAINCALL_SERVICE_NESTED
	li	a1, E5_MODE_NORMAL
	e5_clear_request_tail
	jalr	ra, s0, 0
	bnez	a0, .Le5_outer_normal_fail
	li	t0, E5_CHILD_MAGIC
	bne	a1, t0, .Le5_outer_normal_fail
	e5_call_abi_info
	bnez	a0, .Le5_outer_normal_fail
	li	t0, E5_MAINCALL_ABI_VERSION
	bne	a1, t0, .Le5_outer_normal_fail
	li	a0, E5_OUTER_NORMAL_MAGIC
	e5_restore_frame
.Le5_outer_normal_fail:
	li	a0, -E5_EINVAL
	e5_restore_frame
	.size e5_outer_normal, .-e5_outer_normal

	.globl e5_outer_fault
	.type e5_outer_fault, @function
e5_outer_fault:
	e5_save_frame
	mv	s0, a0
	li	a0, E5_MAINCALL_SERVICE_NESTED
	li	a1, E5_MODE_FAULT
	e5_clear_request_tail
	jalr	ra, s0, 0
	li	t0, -E5_EFAULT
	bne	a0, t0, .Le5_outer_fault_fail
	bnez	a1, .Le5_outer_fault_fail
	e5_call_abi_info
	bnez	a0, .Le5_outer_fault_fail
	li	t0, E5_MAINCALL_ABI_VERSION
	bne	a1, t0, .Le5_outer_fault_fail
	li	a0, E5_OUTER_FAULT_MAGIC
	e5_restore_frame
.Le5_outer_fault_fail:
	li	a0, -E5_EINVAL
	e5_restore_frame
	.size e5_outer_fault, .-e5_outer_fault

	.globl e5_outer_depth
	.type e5_outer_depth, @function
e5_outer_depth:
	e5_save_frame
	mv	s0, a0
	li	a0, E5_MAINCALL_SERVICE_NESTED
	li	a1, E5_MODE_DEPTH
	li	a2, 2
	li	a3, 0
	li	a4, 0
	li	a5, 0
	li	a6, 0
	li	a7, 0
	jalr	ra, s0, 0
	bnez	a0, .Le5_outer_depth_fail
	li	t0, E5_DEPTH_MAGIC
	bne	a1, t0, .Le5_outer_depth_fail
	li	a0, E5_OUTER_DEPTH_MAGIC
	e5_restore_frame
.Le5_outer_depth_fail:
	li	a0, -E5_EINVAL
	e5_restore_frame
	.size e5_outer_depth, .-e5_outer_depth

	.globl e5_child_normal
	.type e5_child_normal, @function
e5_child_normal:
	e5_save_frame
	mv	s0, a0
	e5_call_abi_info
	bnez	a0, .Le5_child_normal_fail
	li	t0, E5_MAINCALL_ABI_VERSION
	bne	a1, t0, .Le5_child_normal_fail
	li	a0, E5_CHILD_MAGIC
	e5_restore_frame
.Le5_child_normal_fail:
	li	a0, -E5_EINVAL
	e5_restore_frame
	.size e5_child_normal, .-e5_child_normal

	.globl e5_child_fault
	.type e5_child_fault, @function
e5_child_fault:
	sd	zero, 0(a0)
	li	a0, E5_POST_FAULT_MAGIC
	ret
	.size e5_child_fault, .-e5_child_fault

	.globl e5_child_depth
	.type e5_child_depth, @function
e5_child_depth:
	e5_save_frame
	sd	s1, 8(sp)
	mv	s0, a0
	mv	s1, a1
	li	a0, E5_MAINCALL_SERVICE_NESTED
	li	a1, E5_MODE_DEPTH
	beqz	s1, .Le5_child_depth_request
	addi	a2, s1, -1
	j	.Le5_child_depth_args
.Le5_child_depth_request:
	li	a2, 0
.Le5_child_depth_args:
	li	a3, 0
	li	a4, 0
	li	a5, 0
	li	a6, 0
	li	a7, 0
	jalr	ra, s0, 0
	beqz	s1, .Le5_child_depth_overflow
	bnez	a0, .Le5_child_depth_fail
	li	t0, E5_DEPTH_MAGIC
	bne	a1, t0, .Le5_child_depth_fail
	j	.Le5_child_depth_pass
.Le5_child_depth_overflow:
	li	t0, -E5_EOVERFLOW
	bne	a0, t0, .Le5_child_depth_fail
	bnez	a1, .Le5_child_depth_fail
.Le5_child_depth_pass:
	li	a0, E5_DEPTH_MAGIC
	ld	s1, 8(sp)
	e5_restore_frame
.Le5_child_depth_fail:
	li	a0, -E5_EINVAL
	ld	s1, 8(sp)
	e5_restore_frame
	.size e5_child_depth, .-e5_child_depth
