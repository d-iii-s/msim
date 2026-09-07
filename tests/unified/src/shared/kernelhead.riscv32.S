.text

/* Leave space for stack */
.org 0x1000
la sp, 0x80001000

/* Reset bare-mode */
csrw satp, zero

/* Jump to C */
jal ra, kernel_wrap
nop

