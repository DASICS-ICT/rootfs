// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>

#include "f1_utmod.h"

asm(
".pushsection .text\n"
".balign 4\n"
".global f1_sleep_then_abi\n"
".type f1_sleep_then_abi, @function\n"
"f1_sleep_then_abi:\n"
"addi sp, sp, -32\n"
"sd ra, 24(sp)\n"
"sd s0, 16(sp)\n"
"sd s1, 8(sp)\n"
"mv s0, a0\n"
"li tp, 1\n"
"li a0, 0x44424702\n"
"li a1, 0\n"
"li a2, 0\n"
"li a3, 0\n"
"li a4, 0\n"
"li a5, 0\n"
"li a6, 0\n"
"li a7, 0\n"
"jalr ra, s0, 0\n"
"bnez a0, 1f\n"
"mv s1, a1\n"
"li a0, 1\n"
"li a1, 1\n"
"li a2, 0\n"
"li a3, 0\n"
"li a4, 0\n"
"li a5, 0\n"
"li a6, 0\n"
"li a7, 0\n"
"jalr ra, s0, 0\n"
"bnez a0, 1f\n"
"li t0, 1\n"
"bne a1, t0, 2f\n"
"li a0, 0\n"
"mv a1, s1\n"
"j 3f\n"
"1:\n"
"li a1, 0\n"
"j 3f\n"
"2:\n"
"li a0, -5\n"
"li a1, 0\n"
"3:\n"
"ld s1, 8(sp)\n"
"ld s0, 16(sp)\n"
"ld ra, 24(sp)\n"
"addi sp, sp, 32\n"
"ret\n"
".size f1_sleep_then_abi, .-f1_sleep_then_abi\n"
".global f1_clobber_return_context\n"
".type f1_clobber_return_context, @function\n"
"f1_clobber_return_context:\n"
"mv sp, a0\n"
"li gp, 1\n"
"li tp, 1\n"
"li s1, 1\n"
"li a0, 0x51\n"
"li a1, 0x52\n"
"ret\n"
".size f1_clobber_return_context, .-f1_clobber_return_context\n"
".global f1_clobber_then_fault\n"
".type f1_clobber_then_fault, @function\n"
"f1_clobber_then_fault:\n"
"mv t3, a0\n"
"li sp, 1\n"
"li gp, 1\n"
"li tp, 1\n"
"li s1, 1\n"
"ld a0, 0(t3)\n"
"ret\n"
".size f1_clobber_then_fault, .-f1_clobber_then_fault\n"
".popsection\n");

EXPORT_SYMBOL(f1_sleep_then_abi);
EXPORT_SYMBOL(f1_clobber_return_context);
EXPORT_SYMBOL(f1_clobber_then_fault);

static int __init f1_utmod_init(void)
{
	return 0;
}

static void __exit f1_utmod_exit(void)
{
}

module_init(f1_utmod_init);
module_exit(f1_utmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS F1 untrusted sleepable-maincall targets");
