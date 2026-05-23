#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x380
    .long 0x90000000     /* WDT interrupt handler */

.section .text
    .org 0x400
_start:
    ldc         r0, sr

    mov.l       wdt_tcsr_address, r0
    /* Period is 8192 cycles and interrupt is selected */
    mov.w       wdt_tcsr_value, r1
    mov.w       r1, @r0

loop:
    cmp/pl      r9
    bf          loop

    cpu_dump
    ehalt


wdt_tcsr_address:
    .long       0xFFFFEC10

wdt_tcsr_value:
    .word       0xA53F

