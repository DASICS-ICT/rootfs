    .section .ulibtext, "ax", @progbits
    .align 2

    .global sreg_untrusted_save_then_use
    .type sreg_untrusted_save_then_use, @function
sreg_untrusted_save_then_use:
    addi sp, sp, -96
    sd s1, 0(sp)
    sd s2, 8(sp)
    sd s3, 16(sp)
    sd s4, 24(sp)
    sd s5, 32(sp)
    sd s6, 40(sp)
    sd s7, 48(sp)
    sd s8, 56(sp)
    sd s9, 64(sp)
    sd s10, 72(sp)
    sd s11, 80(sp)

    li s1, 0x111
    li s2, 0x222
    li s3, 0x333
    li s4, 0x444
    li s5, 0x555
    li s6, 0x666
    li s7, 0x777
    li s8, 0x888
    li s9, 0x999
    li s10, 0xaaa
    li s11, 0xbbb

    ld s1, 0(sp)
    ld s2, 8(sp)
    ld s3, 16(sp)
    ld s4, 24(sp)
    ld s5, 32(sp)
    ld s6, 40(sp)
    ld s7, 48(sp)
    ld s8, 56(sp)
    ld s9, 64(sp)
    ld s10, 72(sp)
    ld s11, 80(sp)
    addi sp, sp, 96
    ret

    .global sreg_untrusted_overwrite
    .type sreg_untrusted_overwrite, @function
sreg_untrusted_overwrite:
    li s1, 0x1111111111111111
    li s2, 0x2222222222222222
    li s3, 0x3333333333333333
    li s4, 0x4444444444444444
    li s5, 0x5555555555555555
    li s6, 0x6666666666666666
    li s7, 0x7777777777777777
    li s8, 0x8888888888888888
    li s9, 0x9999999999999999
    li s10, 0xaaaaaaaaaaaaaaaa
    li s11, 0xbbbbbbbbbbbbbbbb
    ret
