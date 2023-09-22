/*
 * Print hello to the screen and terminate.
 */

.text
.set noat
.set noreorder
.ent __start
__start:
	nop
	/*
	 * Try to store to 0x88000000
	 */
	la $a0, 0x88000004
	lw $a1, 0($a0)
	la $a2, 0xff
	sw $a2, 0($a0)
	nop
	nop
	nop
	nop

	/*
	 * Terminate.
	 */
	.insn
	.word 0x28
	nop
.end __start
