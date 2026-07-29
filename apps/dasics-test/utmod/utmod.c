// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>

#include <asm/kdasics.h>

#include "utmod.h"

MODULE_LICENSE("GPL");

asm(
".pushsection .text\n"
".balign 4\n"
".global c2_test_start\n"
"c2_test_start:\n"
".global c2_test_call0\n"
".type c2_test_call0, @function\n"
"c2_test_call0:\n"
"li a0, 0x10\n"
"li a1, 0x11\n"
"ret\n"
".size c2_test_call0, .-c2_test_call0\n"
".global c2_test_call1\n"
".type c2_test_call1, @function\n"
"c2_test_call1:\n"
"addi a1, a0, 1\n"
"ret\n"
".size c2_test_call1, .-c2_test_call1\n"
".global c2_test_call2\n"
".type c2_test_call2, @function\n"
"c2_test_call2:\n"
"add a0, a0, a1\n"
"addi a1, a0, 2\n"
"ret\n"
".size c2_test_call2, .-c2_test_call2\n"
".global c2_test_call7\n"
".type c2_test_call7, @function\n"
"c2_test_call7:\n"
"add a0, a0, a1\n"
"add a0, a0, a2\n"
"add a0, a0, a3\n"
"add a0, a0, a4\n"
"add a0, a0, a5\n"
"add a0, a0, a6\n"
"andi t0, sp, 15\n"
"bnez t0, 1f\n"
"mv t0, s1\n"
"li s1, 0x55\n"
"mv s1, t0\n"
"mv a1, a6\n"
"ret\n"
"1:\n"
"li a1, -1\n"
"ret\n"
".size c2_test_call7, .-c2_test_call7\n"
".global c2_test_end\n"
"c2_test_end:\n"
".popsection\n");

extern void c2_test_start(void);
extern void c2_test_call0(void);
extern void c2_test_call1(void);
extern void c2_test_call2(void);
extern void c2_test_call7(void);
extern void c2_test_end(void);

EXPORT_SYMBOL(c2_test_start);
EXPORT_SYMBOL(c2_test_call0);
EXPORT_SYMBOL(c2_test_call1);
EXPORT_SYMBOL(c2_test_call2);
EXPORT_SYMBOL(c2_test_call7);
EXPORT_SYMBOL(c2_test_end);

int test_bound(u64 *array)
{
	array[0] = 3;
	array[1] = 4;
	array[2] = 5;
	return 0;
}
EXPORT_SYMBOL(test_bound);

static int init_utmod(void)
{
	return 0;
}

static void exit_utmod(void)
{
}

/*
 * Preserve the KSplit driver->kernel printk boundary fixture without making
 * loader init/exit depend on the removed PoC printf-style maincall ABI.
 * Runtime DASICS tests never authorize or invoke this function.
 */
static noinline __used void ksplit_printk_boundary_fixture(void)
{
	pr_notice("utmod KSplit boundary fixture\n");
}

module_init(init_utmod);
module_exit(exit_utmod);
