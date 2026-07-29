// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>

#include "d3_utmod.h"

u64 d3_touch_cells(struct d3_cell *cells, u64 count)
{
	u64 sum = 0;
	u64 i;

	for (i = 0; i < count; i++)
		cells[i].value = i + 1;
	for (i = count; i > 0; i--)
		sum += cells[i - 1].value;
	return sum;
}
EXPORT_SYMBOL(d3_touch_cells);

int d3_write_one(u64 *value)
{
	*value = 0xd3;
	return 0;
}
EXPORT_SYMBOL(d3_write_one);

static int __init d3_utmod_init(void)
{
	return 0;
}

static void __exit d3_utmod_exit(void)
{
}

module_init(d3_utmod_init);
module_exit(d3_utmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS D3 untrusted refill targets");
