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
 */

#define status 12
#define bad_status 0x18

.text
.set noat
.set noreorder
.ent __start
__start:
	nop
	la $a0, bad_status
	mtc0 $a0, $status
	nop

	/*
	 * So far, good. Now access the printer and
	 * try to print "\n" to it.
	 */
	la $a0, 0x90000000
	la $a1, 0x0A
	sw $a1, 0($a0)
	nop

	/*
	 * If all is okay, terminate.
	 */
	.insn
	.word 0x28
	nop
.end __start

