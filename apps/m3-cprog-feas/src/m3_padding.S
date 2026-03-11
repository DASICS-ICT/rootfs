    .section .ulibtext, "ax", @progbits
    .align 2
    .option push
    .option norvc

    .global m3_untrusted_padding
    .type m3_untrusted_padding, @function
m3_untrusted_padding:
    nop
    nop
    nop
    nop
    ret

    .option pop
