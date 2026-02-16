    .text
    .align 2

    .global sreg_set_pattern_asm
    .type sreg_set_pattern_asm, @function
sreg_set_pattern_asm:
    addi s1, a0, 0x11
    addi s2, a0, 0x22
    addi s3, a0, 0x33
    addi s4, a0, 0x44
    addi s5, a0, 0x55
    addi s6, a0, 0x66
    addi s7, a0, 0x77
    addi s8, a0, 0x88
    addi s9, a0, 0x99
    addi s10, a0, 0xaa
    addi s11, a0, 0xbb
    ret

    .global sreg_snapshot_asm
    .type sreg_snapshot_asm, @function
sreg_snapshot_asm:
    sd s1, 0(a0)
    sd s2, 8(a0)
    sd s3, 16(a0)
    sd s4, 24(a0)
    sd s5, 32(a0)
    sd s6, 40(a0)
    sd s7, 48(a0)
    sd s8, 56(a0)
    sd s9, 64(a0)
    sd s10, 72(a0)
    sd s11, 80(a0)
    ret

    .global sreg_restore_asm
    .type sreg_restore_asm, @function
sreg_restore_asm:
    ld s1, 0(a0)
    ld s2, 8(a0)
    ld s3, 16(a0)
    ld s4, 24(a0)
    ld s5, 32(a0)
    ld s6, 40(a0)
    ld s7, 48(a0)
    ld s8, 56(a0)
    ld s9, 64(a0)
    ld s10, 72(a0)
    ld s11, 80(a0)
    ret
