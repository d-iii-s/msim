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
	 * Printer address is in $a0,
	 * individual letters will be in $a1.
	 */
	la $a0, 0x90000000
	la $a1, 0x48
	sw $a1, 0($a0)
	la $a1, 0x65
	sw $a1, 0($a0)
	la $a1, 0x6c
	sw $a1, 0($a0)
	la $a1, 0x6c
	sw $a1, 0($a0)
	la $a1, 0x6f
	sw $a1, 0($a0)
	la $a1, 0x21
	sw $a1, 0($a0)
	la $a1, 0x0A
	sw $a1, 0($a0)
	nop

	/*
	 * Terminate.
	 */	
	.insn
	.word 0x28
	nop
.end __start
