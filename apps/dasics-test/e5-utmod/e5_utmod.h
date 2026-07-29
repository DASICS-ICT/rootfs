/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _E5_UTMOD_H
#define _E5_UTMOD_H

#define E5_MODE_NORMAL			1
#define E5_MODE_FAULT			2
#define E5_MODE_DEPTH			3

#define E5_CHILD_MAGIC			0xE5000001
#define E5_OUTER_NORMAL_MAGIC		0xE5000002
#define E5_OUTER_FAULT_MAGIC		0xE5000003
#define E5_DEPTH_MAGIC			0xE5000004
#define E5_OUTER_DEPTH_MAGIC		0xE5000005
#define E5_POST_FAULT_MAGIC		0xE50000FF

#define E5_MAINCALL_SERVICE_ABI_INFO	1
#define E5_MAINCALL_QUERY_RUNTIME	1
#define E5_MAINCALL_ABI_VERSION		1
#define E5_MAINCALL_SERVICE_NESTED	0x44424701

#define E5_EINVAL			22
#define E5_EFAULT			14
#define E5_EOVERFLOW			75

#ifndef __ASSEMBLY__
long e5_outer_normal(void *maincall);
long e5_outer_fault(void *maincall);
long e5_outer_depth(void *maincall);
long e5_child_normal(void *maincall);
long e5_child_fault(unsigned long *target);
long e5_child_depth(void *maincall, unsigned long remaining);
#endif

#endif
