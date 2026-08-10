#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x28
    .long _dmac_error_handler /* DMAC address error handler */

    .org 0x120
    .long _dmac_handler  /* DMAC channel 0 interrupt handler */

    .org 0x128
    .long _dmac_handler  /* DMAC channel 1 interrupt handler */

    .org 0x130
    .long _dmac_handler  /* DMAC channel 2 interrupt handler */

    .org 0x138
    .long _dmac_handler  /* DMAC channel 3 interrupt handler */

.section .text
    .org 0x400
_start:
    ldc         r0, sr
    mov         #2, r8

    /* Write the values to source addresses */
    mov         #1, r0
    mov.l       source_address_0, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1

    add         #1, r0
    mov.l       source_address_2, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1
    add         #1, r0
    add         #4, r1
    mov.l       r0, @r1

    /* Prepare the DMAC channel 0 */
    mov.l       source_address_0, r10
    mov.l       dmac_sar0_address, r0
    mov.l       r10, @r0
    mov.l       destination_address_0, r10
    mov.l       dmac_dar0_address, r0
    mov.l       r10, @r0
    /* 4 transfers */
    mov         #4, r1
    mov.l       dmac_dmatcr0_address, r0
    mov.l       r1, @r0
    mov.l       dmac_chcr0_value, r1
    mov.l       dmac_chcr0_address, r0
    mov.l       r1, @r0

    /* Prepare the DMAC channel 2 */
    mov.l       source_address_2, r11
    mov.l       dmac_sar2_address, r0
    mov.l       r11, @r0
    mov.l       destination_address_2, r11
    mov.l       dmac_dar2_address, r0
    mov.l       r11, @r0
    /* 4 transfers */
    mov         #4, r1
    mov.l       dmac_dmatcr2_address, r0
    mov.l       r1, @r0
    mov.l       dmac_chcr2_value, r1
    mov.l       dmac_chcr2_address, r0
    mov.l       r1, @r0

    /* Enable the DMAC */
    mov         #1, r0
    mov.l       dmac_dmaor_address, r1
    mov.w       r0, @r1

main_loop:
    cmp/hs      r8, r9
    bf          main_loop
    nop

    /* Load the SAR registers into CPU register, both should be the same as the channel 2 has auto-reload on */
    mov.l       dmac_sar0_address, r2
    mov.l       @r2, r3
    mov.l       dmac_sar2_address, r6
    mov.l       @r6, r7
    cpu_dump
    /* Check if every value was correctly written into memory */
    mov.l       @r10+, r0
    mov.l       @r11+, r1
    cpu_dump
    mov.l       @r10+, r0
    mov.l       @r11+, r1
    cpu_dump
    mov.l       @r10+, r0
    mov.l       @r11+, r1
    cpu_dump
    mov.l       @r10+, r0
    mov.l       @r11+, r1
    cpu_dump
    ehalt

_dmac_handler:
    mov     #0, r0
    add     #1, r9
    ldc     r0, sr
    bra     main_loop
    nop

_dmac_error_handler:
    mov     #2, r9
    bra     main_loop
    nop

.align 2
dmac_sar0_address:
    .long       0xFFFFECC0

dmac_dar0_address:
    .long       0xFFFFECC4

dmac_dmatcr0_address:
    .long       0xFFFFECC8

dmac_chcr0_address:
    .long       0xFFFFECCC

dmac_sar2_address:
    .long       0xFFFFECE0

dmac_dar2_address:
    .long       0xFFFFECE4

dmac_dmatcr2_address:
    .long       0xFFFFECE8

dmac_chcr2_address:
    .long       0xFFFFECEC

dmac_dmaor_address:
    .long       0xFFFFECB0

/* 00 - direct access, do not reload */
/* 1F - auto request */
/* 11 - increment both source address and destination address after transfer */
/* 25 - longword size, cycle-steal, interrupt generated, enabled channel */
dmac_chcr0_value:
    .long       0x001F1125

/* 00 - direct access, reload-on */
/* 1F - auto request */
/* 11 - increment both source address and destination address after transfer */
/* 25 - longword size, cycle-steal, interrupt generated, enabled channel */
dmac_chcr2_value:
    .long       0x011F1125

source_address_0:
    .long       0xFFFFA000

source_address_2:
    .long       0xFFFFA010

destination_address_0:
    .long       0xFFFFB000

destination_address_2:
    .long       0xFFFFB010