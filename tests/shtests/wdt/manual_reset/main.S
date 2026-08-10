#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x8
    .long _manual_reset   /* Manual reset - PC */

    .org 0xC
    .long 0xFFFF8000     /* Manual reset - SP */

.section .text
    .org 0x400
_start:
    ldc         r0, sr

    mov.l       wdt_rstcsr_address, r0
    /* Selected manual reset */
    mov.w       wdt_rstcsr_value, r1
    mov.w       r1, @r0

    mov.l       wdt_tcsr_address, r0
    /* Period is 8 cycles and watchdog timer mode is selected */
    mov.w       wdt_tcsr_value, r1
    mov.w       r1, @r0

loop:
    cmp/pl      r9
    bf          loop
    nop

    cpu_dump
    ehalt

_manual_reset:
    mov     #1, r9
    bra     _start
    nop


.align 2
wdt_tcsr_address:
    .long       0xFFFFEC10

wdt_rstcsr_address:
    .long       0xFFFFEC12

wdt_tcsr_value:
    .word       0xA578

wdt_rstcsr_value:
    .word       0x5A7F


