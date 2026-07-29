/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _E1_UTMOD_H
#define _E1_UTMOD_H

int e1_valid_service(void);
int e1_full_registers(void);
int e1_unknown_service(void);
int e1_bad_arguments(void);
int e1_invalid_source(void);
int e1_untrusted_stack(void);

#endif
