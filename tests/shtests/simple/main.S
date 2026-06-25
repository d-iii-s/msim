.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

.section .text
    .org 0x400
_start:
    mov.l printer_address, r0
    mov.l data_address_1, r1
    mov.l data_address_2, r2
    mov.l @r1, r3
    mov.l @r2, r4
    add r3, r4

    mov.l r4, @r0
    .word 0x8200

printer_address:
    .long 0x90000000

data_address_1:
    .long 0xF0000000

data_address_2:
    .long 0xF0000004