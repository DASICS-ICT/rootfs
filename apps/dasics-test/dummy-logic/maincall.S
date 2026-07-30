/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/linkage.h>

	.section .rodata, "a"
	.align 3
.Ldummy_maincall_entry:
	/*
	 * Use an absolute data relocation: untrusted and trusted module
	 * mappings are too far apart for a RISC-V PC-relative call.
	 */
	.dword	dasics_maincall_gate

	.text
	.align 2
ENTRY(dummy_maincall)
	addi	sp, sp, -16
	sd	ra, 8(sp)
	/*
	 * The untrusted module mapping is more than 2 GiB away from the
	 * trusted kernel, so a normal module relocation cannot address the
	 * gate. The loader resolves the read-only absolute pointer above.
	 */
	la	t0, .Ldummy_maincall_entry
	ld	t0, 0(t0)
	jalr	ra, t0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
ENDPROC(dummy_maincall)
