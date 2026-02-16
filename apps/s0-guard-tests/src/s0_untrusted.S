    .section .ulibtext, "ax", @progbits
    .align 2
    .option push
    .option norvc

    .global s0_untrusted_padding
    .type s0_untrusted_padding, @function
s0_untrusted_padding:
    nop
    ret

    .global s0_untrusted_viol_write
    .type s0_untrusted_viol_write, @function
s0_untrusted_viol_write:
    li s0, 0x123
    ret

    .global s0_untrusted_viol_read
    .type s0_untrusted_viol_read, @function
s0_untrusted_viol_read:
    addi a0, s0, 1
    ret

    .global s0_untrusted_proto_mismatch
    .type s0_untrusted_proto_mismatch, @function
s0_untrusted_proto_mismatch:
    addi sp, sp, -16
    sd s0, 0(sp)
    addi s0, s0, 1
    ld s0, 8(sp)
    addi sp, sp, 16
    ret

    .global s0_untrusted_legal_save_restore
    .type s0_untrusted_legal_save_restore, @function
s0_untrusted_legal_save_restore:
    addi sp, sp, -16
    sd s0, 0(sp)
    addi s0, s0, 1
    ld s0, 0(sp)
    addi sp, sp, 16
    ret

    .option pop
