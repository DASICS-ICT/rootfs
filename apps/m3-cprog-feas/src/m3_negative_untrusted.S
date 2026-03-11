    .section .ulibtext, "ax", @progbits
    .align 2
    .option push
    .option norvc

    .global m3_untrusted_process_case20_auth_tamper_bitflip_c
    .type m3_untrusted_process_case20_auth_tamper_bitflip_c, @function
m3_untrusted_process_case20_auth_tamper_bitflip_c:
    addi sp, sp, -16
    sd s1, 0(sp)
    li s1, 7
    ld t0, 0(sp)
    xori t0, t0, 1
    sd t0, 0(sp)
    ld s1, 0(sp)
    li a0, 0
    addi sp, sp, 16
    ret

    .global m3_untrusted_process_case21_auth_tamper_overwrite_c
    .type m3_untrusted_process_case21_auth_tamper_overwrite_c, @function
m3_untrusted_process_case21_auth_tamper_overwrite_c:
    addi sp, sp, -16
    sd s1, 0(sp)
    li s1, 7
    li t0, -1
    sd t0, 0(sp)
    ld s1, 0(sp)
    li a0, 0
    addi sp, sp, 16
    ret

    .global m3_untrusted_process_case22_auth_tamper_overflow_c
    .type m3_untrusted_process_case22_auth_tamper_overflow_c, @function
m3_untrusted_process_case22_auth_tamper_overflow_c:
    addi sp, sp, -16
    sd s1, 0(sp)
    li s1, 7
    mv a0, sp
    call m3_untrusted_case22_overflow_helper
    ld s1, 0(sp)
    li a0, 0
    addi sp, sp, 16
    ret

    .option pop
