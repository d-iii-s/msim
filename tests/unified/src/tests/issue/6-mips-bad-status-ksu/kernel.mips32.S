/*
 * Check that invalid status register does not terminate
 * the simulator. We set KSU bits to 0b11 (invalid value).
 *
 * This is a regression from 1.3.8.5 where no special action
 * was taken, in 1878309139944b78b06 MSIM fails with assertion.
 *
 * Also see issue #6 https://github.com/D-iii-S/msim/issues/6
 * "Invalid status register shall not terminate simulation".
 *
 * According to manual, KSU could be 0b11 but EXL or ERL must
 * be set to 1, otherwise CPU would not be in kernel mode
 * (but because KSU is not 0b01 or 0b10, it will not be in
 * user or supervisor either).
 *
 * So when the whole status register is set to KSU 0b11, the
 * behaviour is probably undefined?
 *
 * MSIM raises address error as we do not know which memory
 * (if any) restrictions to apply. Raising coprocessor unusable
 * might be an option, probably?
 */

#define status 12
#define bad_status 0x18

.text
.set noreorder

.globl kernel_main
.ent kernel_main
kernel_main:
    nop
	la $a0, bad_status
	mtc0 $a0, $status

    /* If we get here, we only return. */
	nop
    j $ra
    nop

.end kernel_main

