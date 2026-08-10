#define ehalt .word 0x8200

.section .vectors, "ax"
    .long __start          /* Reset PC */
    .long 0xFFFFD000     /* Reset SP */

.section __start
.align 2
start:
    mov.l main_addr, r0
    jsr @r0
    nop
    ehalt

.align 2
main_addr:
    .long _main