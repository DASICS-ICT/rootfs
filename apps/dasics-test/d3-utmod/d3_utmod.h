/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _D3_UTMOD_H
#define _D3_UTMOD_H

#include <linux/types.h>

#define D3_NR_CELLS 20

struct d3_cell {
	u64 value;
	u64 gap;
};

u64 d3_touch_cells(struct d3_cell *cells, u64 count);
int d3_write_one(u64 *value);

#endif
