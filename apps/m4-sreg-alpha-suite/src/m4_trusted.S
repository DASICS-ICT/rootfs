    .text
    .align 2
    .option push
    .option norvc

    .global m4_trusted_call_s1_readback
    .type m4_trusted_call_s1_readback, @function
m4_trusted_call_s1_readback:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s1, 8(sp)
    mv s1, a1
    mv t0, a0
    .word 0x0002908b
    mv a0, s1
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_trusted_call_s1_no_readback
    .type m4_trusted_call_s1_no_readback, @function
m4_trusted_call_s1_no_readback:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s1, 8(sp)
    mv s1, a1
    mv t0, a0
    .word 0x0002908b
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_trusted_call_s1_overwrite_readback
    .type m4_trusted_call_s1_overwrite_readback, @function
m4_trusted_call_s1_overwrite_readback:
    addi sp, sp, -24
    sd ra, 0(sp)
    sd s1, 8(sp)
    sd a2, 16(sp)
    mv s1, a1
    mv t0, a0
    .word 0x0002908b
    ld a2, 16(sp)
    mv s1, a2
    mv a0, s1
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 24
    ret

    .global m4_trusted_call_s1_fault_readback
    .type m4_trusted_call_s1_fault_readback, @function
m4_trusted_call_s1_fault_readback:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s1, 8(sp)
    mv s1, a1
    mv t0, a0
    .word 0x0002908b
    mv a0, s1
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_trusted_call_s2_fault_readback
    .type m4_trusted_call_s2_fault_readback, @function
m4_trusted_call_s2_fault_readback:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s2, 8(sp)
    mv s2, a1
    mv t0, a0
    .word 0x0002908b
    mv a0, s2
    ld s2, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_trusted_call_s11_fault_readback
    .type m4_trusted_call_s11_fault_readback, @function
m4_trusted_call_s11_fault_readback:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s11, 8(sp)
    mv s11, a1
    mv t0, a0
    .word 0x0002908b
    mv a0, s11
    ld s11, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_trusted_call_s1_s2_fault_readback
    .type m4_trusted_call_s1_s2_fault_readback, @function
m4_trusted_call_s1_s2_fault_readback:
    addi sp, sp, -24
    sd ra, 0(sp)
    sd s1, 8(sp)
    sd s2, 16(sp)
    mv s1, a1
    mv s2, a2
    mv t0, a0
    .word 0x0002908b
    mv a0, s2
    ld s2, 16(sp)
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 24
    ret

    .global m4_trusted_call_s1_s11_fault_readback
    .type m4_trusted_call_s1_s11_fault_readback, @function
m4_trusted_call_s1_s11_fault_readback:
    addi sp, sp, -24
    sd ra, 0(sp)
    sd s1, 8(sp)
    sd s11, 16(sp)
    mv s1, a1
    mv s11, a2
    mv t0, a0
    .word 0x0002908b
    mv a0, s11
    ld s11, 16(sp)
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 24
    ret

    .global m4_trusted_call_s1_s11_readback
    .type m4_trusted_call_s1_s11_readback, @function
m4_trusted_call_s1_s11_readback:
    addi sp, sp, -40
    sd ra, 0(sp)
    sd s1, 8(sp)
    sd s11, 16(sp)
    sd a3, 24(sp)
    sd a4, 32(sp)
    mv s1, a1
    mv s11, a2
    mv t0, a0
    .word 0x0002908b
    ld a3, 24(sp)
    ld a4, 32(sp)
    mv t0, s1
    mv t1, s11
    sd t0, 0(a3)
    sd t1, 0(a4)
    ld s11, 16(sp)
    ld s1, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 40
    ret

    .global m4_trusted_call_safe_return
    .type m4_trusted_call_safe_return, @function
m4_trusted_call_safe_return:
    addi sp, sp, -104
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    sd s6, 56(sp)
    sd s7, 64(sp)
    sd s8, 72(sp)
    sd s9, 80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)
    mv s1, a1
    mv s11, a2
    mv t0, a0
    .word 0x0002908b
    ld s11, 96(sp)
    ld s10, 88(sp)
    ld s9, 80(sp)
    ld s8, 72(sp)
    ld s7, 64(sp)
    ld s6, 56(sp)
    ld s5, 48(sp)
    ld s4, 40(sp)
    ld s3, 32(sp)
    ld s2, 24(sp)
    ld s1, 16(sp)
    ld s0, 8(sp)
    ld ra, 0(sp)
    addi sp, sp, 104
    ret

    .option pop
