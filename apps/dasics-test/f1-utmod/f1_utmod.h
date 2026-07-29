/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _F1_UTMOD_H
#define _F1_UTMOD_H

unsigned long f1_sleep_then_abi(unsigned long maincall);
unsigned long f1_clobber_return_context(unsigned long attacker_sp);
unsigned long f1_clobber_then_fault(const unsigned long *address);

#endif
