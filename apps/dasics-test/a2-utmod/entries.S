#include <asm/asm.h>
#include <asm/csr.h>

#define A2_REQUEST_CALLBACK 1
#define A2_REPORT_CALLBACK  2
#define A2_REPORT_OUTER     3

	.section .text.a2_nested, "ax"
	.align 2
	.globl a2_untrusted_start
a2_untrusted_start:

	.globl a2_outer
	.type a2_outer, @function
a2_outer:
	addi sp, sp, -16
	sd ra, 8(sp)
	csrr t0, CSR_DMAINCALL
	li a0, A2_REQUEST_CALLBACK
	la a1, a2_callback
	jalr ra, t0, 0
	csrr t0, CSR_DMAINCALL
	li a0, A2_REPORT_OUTER
	li a1, 0
	jalr ra, t0, 0
	ld ra, 8(sp)
	addi sp, sp, 16
	li a0, 0
	ret
	.size a2_outer, .-a2_outer

	.align 2
	.globl a2_callback
	.type a2_callback, @function
a2_callback:
	addi sp, sp, -16
	sd ra, 8(sp)
	csrr t0, CSR_DMAINCALL
	li a0, A2_REPORT_CALLBACK
	li a1, 0
	jalr ra, t0, 0
	ld ra, 8(sp)
	addi sp, sp, 16
	li a0, 0
	ret
	.size a2_callback, .-a2_callback

	.globl a2_untrusted_end
a2_untrusted_end:
