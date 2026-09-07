.text
.set noreorder

/* Leave space for stack */
.org 0x1000
la $gp, 0x80000000
addi $sp, $gp, 0x1000

/* Leave BEV and disable interrupts globally. */
la $k0, 0xff00
mtc0 $k0, $12

/* Jump to C */
jal kernel_wrap
nop

