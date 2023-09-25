/*
 * Acess dnomem region.
 *
 * This file is shared among multiple tests.
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
	 *
	 * Will print "read".
	 */
	la $a0, 0x90000000
	la $a1, 0x72
	sw $a1, 0($a0)
	la $a1, 0x65
	sw $a1, 0($a0)
	la $a1, 0x61
	sw $a1, 0($a0)
	la $a1, 0x64
	sw $a1, 0($a0)
	la $a1, 0x0a
	sw $a1, 0($a0)

	/*
	 * Try to read from 0x88000000
	 */
	la $a2, 0x88000004
	lw $a3, 0($a2)

	/*
	 * Will print "write".
	 */
	la $a1, 0x77
	sw $a1, 0($a0)
	la $a1, 0x72
	sw $a1, 0($a0)
	la $a1, 0x69
	sw $a1, 0($a0)
	la $a1, 0x74
	sw $a1, 0($a0)
	la $a1, 0x65
	sw $a1, 0($a0)
	la $a1, 0x0a
	sw $a1, 0($a0)

	/*
	 * Try to write to 0x88000000
	 */
	la $a3, 0xff
	sw $a3, 0($a2)
	nop

	/*
	 * Terminate.
	 */
	.insn
	.word 0x28
	nop
.end __start
