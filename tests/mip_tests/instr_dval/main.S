/*
 * Check that special instruction for dumping CP0 register works.
 */

.text
.set noat
.set noreorder
.ent __start
__start:
	nop
	li $a0, 0xdeadbeef
	nop
	.insn
	.word 0x35
	nop

	/*
	 * Terminate.
	 */	
	.insn
	.word 0x28
	nop
.end __start

