// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>

#include "e5_utmod.h"

EXPORT_SYMBOL(e5_outer_normal);
EXPORT_SYMBOL(e5_outer_fault);
EXPORT_SYMBOL(e5_outer_depth);
EXPORT_SYMBOL(e5_child_normal);
EXPORT_SYMBOL(e5_child_fault);
EXPORT_SYMBOL(e5_child_depth);

static int __init e5_utmod_init(void)
{
	return 0;
}

static void __exit e5_utmod_exit(void)
{
}

module_init(e5_utmod_init);
module_exit(e5_utmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS E5 nested call untrusted targets");
