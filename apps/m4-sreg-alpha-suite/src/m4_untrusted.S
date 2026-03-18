    .section .ulibtext, "ax", @progbits
    .align 2
    .option push
    .option norvc

    .equ TOKEN0_OFF, 0
    .equ TOKEN1_OFF, 8
    .equ TOKEN2_OFF, 16
    .equ FAIL_OFF,   40

    .global m4_untrusted_no_touch
    .type m4_untrusted_no_touch, @function
m4_untrusted_no_touch:
    ret

    .global m4_untrusted_first_read_s1
    .type m4_untrusted_first_read_s1, @function
m4_untrusted_first_read_s1:
    mv t0, s1
    la t1, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t1)
    ret

    .global m4_untrusted_token_roundtrip_s1
    .type m4_untrusted_token_roundtrip_s1, @function
m4_untrusted_token_roundtrip_s1:
    mv t0, s1
    addi sp, sp, -16
    sd s1, 0(sp)
    ld t1, 0(sp)
    la t2, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t2)
    sd t1, TOKEN1_OFF(t2)
    sd s1, TOKEN2_OFF(t2)
    bne t0, t1, .Lroundtrip_fail
    bne s1, t1, .Lroundtrip_fail
    ld s1, 0(sp)
    addi sp, sp, 16
    ret
.Lroundtrip_fail:
    li t3, 1
    sd t3, FAIL_OFF(t2)
    ld s1, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_untrusted_arith_corrupt_s1
    .type m4_untrusted_arith_corrupt_s1, @function
m4_untrusted_arith_corrupt_s1:
    mv t0, s1
    la t1, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t1)
    addi s1, s1, 1
    sd s1, TOKEN1_OFF(t1)
    ret

    .global m4_untrusted_arith_corrupt_s2
    .type m4_untrusted_arith_corrupt_s2, @function
m4_untrusted_arith_corrupt_s2:
    mv t0, s2
    la t1, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t1)
    addi s2, s2, 1
    sd s2, TOKEN1_OFF(t1)
    ret

    .global m4_untrusted_first_dest_write_s1
    .type m4_untrusted_first_dest_write_s1, @function
m4_untrusted_first_dest_write_s1:
    li s1, 0x5a5
    la t0, g_m4_alpha_shared
    sd s1, TOKEN0_OFF(t0)
    ret

    .global m4_untrusted_first_dest_write_s11
    .type m4_untrusted_first_dest_write_s11, @function
m4_untrusted_first_dest_write_s11:
    li s11, 0x5a5
    la t0, g_m4_alpha_shared
    sd s11, TOKEN0_OFF(t0)
    ret

    .global m4_untrusted_swap_s1_to_s2
    .type m4_untrusted_swap_s1_to_s2, @function
m4_untrusted_swap_s1_to_s2:
    mv t0, s1
    la t1, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t1)
    mv s2, t0
    sd s2, TOKEN1_OFF(t1)
    ret

    .global m4_untrusted_swap_s1_to_s11
    .type m4_untrusted_swap_s1_to_s11, @function
m4_untrusted_swap_s1_to_s11:
    mv t0, s1
    la t1, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t1)
    mv s11, t0
    sd s11, TOKEN1_OFF(t1)
    ret

    .global m4_untrusted_multi_slot_independence
    .type m4_untrusted_multi_slot_independence, @function
m4_untrusted_multi_slot_independence:
    mv t0, s1
    mv t1, s11
    la t2, g_m4_alpha_shared
    sd t0, TOKEN0_OFF(t2)
    sd t1, TOKEN1_OFF(t2)
    addi sp, sp, -16
    sd s1, 0(sp)
    sd s11, 8(sp)
    ld s1, 0(sp)
    ld s11, 8(sp)
    addi sp, sp, 16
    ret

    .global m4_untrusted_stack_token_bitflip_s1
    .type m4_untrusted_stack_token_bitflip_s1, @function
m4_untrusted_stack_token_bitflip_s1:
    addi sp, sp, -16
    sd s1, 0(sp)
    la t1, g_m4_alpha_shared
    ld t0, 0(sp)
    sd t0, TOKEN0_OFF(t1)
    xori t0, t0, 1
    sd t0, 0(sp)
    sd t0, TOKEN1_OFF(t1)
    ld s1, 0(sp)
    addi sp, sp, 16
    ret

    .global m4_untrusted_stack_token_overflow_s1
    .type m4_untrusted_stack_token_overflow_s1, @function
m4_untrusted_stack_token_overflow_s1:
    addi sp, sp, -32
    sd s1, 16(sp)
    la t1, g_m4_alpha_shared
    ld t0, 16(sp)
    sd t0, TOKEN0_OFF(t1)
    li t2, -1
    sb t2, 16(sp)
    sb t2, 17(sp)
    sb t2, 18(sp)
    sb t2, 19(sp)
    sb t2, 20(sp)
    sb t2, 21(sp)
    sb t2, 22(sp)
    sb t2, 23(sp)
    ld t0, 16(sp)
    sd t0, TOKEN1_OFF(t1)
    ld s1, 16(sp)
    addi sp, sp, 32
    ret

    .option pop
