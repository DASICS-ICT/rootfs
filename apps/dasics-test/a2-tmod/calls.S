#include <asm/asm.h>

	.text
	.align 2
	.globl a2_call_outer
	.type a2_call_outer, @function
a2_call_outer:
	addi sp, sp, -16
	sd ra, 8(sp)
	.word 0x0005108b
	ld ra, 8(sp)
	addi sp, sp, 16
	ret
	.size a2_call_outer, .-a2_call_outer

	.align 2
	.globl a2_call_callback
	.type a2_call_callback, @function
a2_call_callback:
	addi sp, sp, -16
	sd ra, 8(sp)
	.word 0x0005108b
	ld ra, 8(sp)
	addi sp, sp, 16
	ret
	.size a2_call_callback, .-a2_call_callback
