    .section .ulibtext, "ax", @progbits
    .align 2
    .option push
    .option norvc

    .global sreg_untrusted_padding
    .type sreg_untrusted_padding, @function
sreg_untrusted_padding:
    nop
    ret

    .macro DEFINE_SREG_CASES reg, suffix, imm
    .global sreg_untrusted_viol_write_\suffix
    .type sreg_untrusted_viol_write_\suffix, @function
sreg_untrusted_viol_write_\suffix:
    li \reg, \imm
    ret

    .global sreg_untrusted_viol_read_\suffix
    .type sreg_untrusted_viol_read_\suffix, @function
sreg_untrusted_viol_read_\suffix:
    addi a0, \reg, 1
    ret

    .global sreg_untrusted_proto_mismatch_\suffix
    .type sreg_untrusted_proto_mismatch_\suffix, @function
sreg_untrusted_proto_mismatch_\suffix:
    addi sp, sp, -16
    sd \reg, 0(sp)
    addi \reg, \reg, 1
    ld \reg, 8(sp)
    addi sp, sp, 16
    ret

    .global sreg_untrusted_legal_save_restore_\suffix
    .type sreg_untrusted_legal_save_restore_\suffix, @function
sreg_untrusted_legal_save_restore_\suffix:
    addi sp, sp, -16
    sd \reg, 0(sp)
    addi \reg, \reg, 1
    ld \reg, 0(sp)
    addi sp, sp, 16
    ret

    .global sreg_untrusted_auth_tamper_bitflip_\suffix
    .type sreg_untrusted_auth_tamper_bitflip_\suffix, @function
sreg_untrusted_auth_tamper_bitflip_\suffix:
    addi sp, sp, -16
    sd \reg, 0(sp)
    lbu t0, 0(sp)
    xori t0, t0, 1
    sb t0, 0(sp)
    ld \reg, 0(sp)
    addi sp, sp, 16
    ret

    .global sreg_untrusted_auth_tamper_overwrite_\suffix
    .type sreg_untrusted_auth_tamper_overwrite_\suffix, @function
sreg_untrusted_auth_tamper_overwrite_\suffix:
    addi sp, sp, -16
    sd \reg, 0(sp)
    li t0, 0
    sd t0, 0(sp)
    ld \reg, 0(sp)
    addi sp, sp, 16
    ret
    .endm

    DEFINE_SREG_CASES s0,  s0,  0x123
    DEFINE_SREG_CASES s1,  s1,  0x124
    DEFINE_SREG_CASES s2,  s2,  0x125
    DEFINE_SREG_CASES s3,  s3,  0x126
    DEFINE_SREG_CASES s4,  s4,  0x127
    DEFINE_SREG_CASES s5,  s5,  0x128
    DEFINE_SREG_CASES s6,  s6,  0x129
    DEFINE_SREG_CASES s7,  s7,  0x12a
    DEFINE_SREG_CASES s8,  s8,  0x12b
    DEFINE_SREG_CASES s9,  s9,  0x12c
    DEFINE_SREG_CASES s10, s10, 0x12d
    DEFINE_SREG_CASES s11, s11, 0x12e

    .option pop
