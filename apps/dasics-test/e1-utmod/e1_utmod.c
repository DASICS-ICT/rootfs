// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>

#include "e1_utmod.h"

asm(
".pushsection .text\n"
".balign 4\n"
".macro e1_call name, service, query, last\n"
".global \\name\n"
".type \\name, @function\n"
"\\name:\n"
"addi sp, sp, -16\n"
"sd ra, 8(sp)\n"
"mv t0, a0\n"
"li a0, \\service\n"
"li a1, \\query\n"
"li a2, 0\n"
"li a3, 0\n"
"li a4, 0\n"
"li a5, 0\n"
"li a6, 0\n"
"li a7, \\last\n"
"jalr ra, t0, 0\n"
"ld ra, 8(sp)\n"
"addi sp, sp, 16\n"
"ret\n"
".size \\name, .-\\name\n"
".endm\n"
"e1_call e1_valid_service, 1, 1, 0\n"
".global e1_full_registers\n"
".type e1_full_registers, @function\n"
"e1_full_registers:\n"
"addi sp, sp, -16\n"
"sd ra, 8(sp)\n"
"mv t0, a0\n"
"li a0, 1\n"
"li a1, 2\n"
"li a2, 2\n"
"li a3, 3\n"
"li a4, 4\n"
"li a5, 5\n"
"li a6, 6\n"
"li a7, 7\n"
"jalr ra, t0, 0\n"
"ld ra, 8(sp)\n"
"addi sp, sp, 16\n"
"ret\n"
".size e1_full_registers, .-e1_full_registers\n"
"e1_call e1_unknown_service, 57005, 0, 0\n"
"e1_call e1_bad_arguments, 1, 1, 1\n"
".global e1_invalid_source\n"
".type e1_invalid_source, @function\n"
"e1_invalid_source:\n"
"mv t0, a0\n"
"li a0, 1\n"
"li a1, 1\n"
"li a2, 0\n"
"li a3, 0\n"
"li a4, 0\n"
"li a5, 0\n"
"li a6, 0\n"
"li a7, 0\n"
"li ra, 1\n"
"jr t0\n"
".size e1_invalid_source, .-e1_invalid_source\n"
".global e1_untrusted_stack\n"
".type e1_untrusted_stack, @function\n"
"e1_untrusted_stack:\n"
"addi sp, sp, -32\n"
"sd ra, 24(sp)\n"
"sd s0, 16(sp)\n"
"sd s1, 8(sp)\n"
"mv s0, sp\n"
"mv t0, a0\n"
"mv s1, a1\n"
"li a0, 1\n"
"li a1, 1\n"
"li a2, 0\n"
"li a3, 0\n"
"li a4, 0\n"
"li a5, 0\n"
"li a6, 0\n"
"li a7, 0\n"
"mv sp, s1\n"
"jalr ra, t0, 0\n"
"mv sp, s0\n"
"ld s1, 8(sp)\n"
"ld s0, 16(sp)\n"
"ld ra, 24(sp)\n"
"addi sp, sp, 32\n"
"ret\n"
".size e1_untrusted_stack, .-e1_untrusted_stack\n"
".popsection\n");

EXPORT_SYMBOL(e1_valid_service);
EXPORT_SYMBOL(e1_full_registers);
EXPORT_SYMBOL(e1_unknown_service);
EXPORT_SYMBOL(e1_bad_arguments);
EXPORT_SYMBOL(e1_invalid_source);
EXPORT_SYMBOL(e1_untrusted_stack);

static int __init e1_utmod_init(void)
{
	return 0;
}

static void __exit e1_utmod_exit(void)
{
}

module_init(e1_utmod_init);
module_exit(e1_utmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS E1 untrusted maincall request targets");
