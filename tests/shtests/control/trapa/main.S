#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x80
    .long 0x480          /* TRAPA vector number 1 */
    .long 0x490          /* TRAPA vector number 2 */
    .long 0x500          /* TRAPA vector number 3 */
    .long 0x510          /* TRAPA vector number 4 */
    .long 0x520          /* TRAPA vector number 5 */
    .long 0x530          /* TRAPA vector number 6 */
    .long 0x540          /* TRAPA vector number 7 */
    .long 0x550          /* TRAPA vector number 8 */
    .long 0x560          /* TRAPA vector number 9 */

    .org 0xFC
    .long 0x570          /* TRAPA vector number 32 */

.section .text
    .org 0x400
_start:
    trapa       #0x20
    nop
    trapa       #0x21
    nop
    trapa       #0x22
    nop
    trapa       #0x23
    nop
    trapa       #0x24
    nop
    trapa       #0x25
    nop
    trapa       #0x26
    nop
    trapa       #0x27
    nop
    trapa       #0x28
    nop
    trapa       #0x3F
    nop
    cpu_dump
    ehalt

.section .text
    .org 0x480
    mov         #1, r0
    rte
    nop


.section .text
    .org 0x490
    mov         #1, r1
    rte
    nop

.section .text
    .org 0x500
    mov         #1, r2
    rte
    nop

.section .text
    .org 0x510
    mov         #1, r3
    rte
    nop

.section .text
    .org 0x520
    mov         #1, r4
    rte
    nop

.section .text
    .org 0x530
    mov         #1, r5
    rte
    nop

.section .text
    .org 0x540
    mov         #1, r6
    rte
    nop

.section .text
    .org 0x550
    mov         #1, r7
    rte
    nop


.section .text
    .org 0x560
    mov         #1, r8
    rte
    nop


.section .text
    .org 0x570
    mov         #1, r9
    rte
    nop