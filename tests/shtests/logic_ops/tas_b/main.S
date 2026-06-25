#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

.section .text
    .org 0x400
_start:
    mov.l       data_addr_1, r0
    mov.l       data_addr_2, r1
    // the byte in memory should be 0x00, so the T-bit should be set after this and the MSb should be set in the memory
    tas.b       @r0
    mov.l       @r0, r2 
    cpu_dump
    // the byte in memory should be 0x01, so the T-bit should not be set after this and the MSb should be set in the memory
    tas.b       @r1
    mov.l       @r0, r2
    cpu_dump
    ehalt

.align 2
data_addr_1:
    .long   0xF0000000

data_addr_2:
    .long   0xF0000001