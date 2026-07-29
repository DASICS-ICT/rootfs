/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _D4_UTMOD_H
#define _D4_UTMOD_H

#include <linux/types.h>

u64 d4_fault_load(const u64 *address, u64 *after);
int d4_fault_store(u64 *address, u64 *after);
int d4_fault_jump(void (*target)(void), u64 *after);
int d4_fault_stack_low(u64 *after);
int d4_fault_stack_high(u64 *after);

#endif
