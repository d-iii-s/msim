/*
 * Check that special instruction for dumping CP0 register works.
 */

.text
.set noat
.set noreorder
.ent __start
__start:
	nop
	.word 0x37
	nop
	.word 0x0e
	nop

	/*
	 * Terminate.
	 */
	.insn
	.word 0x28
	nop
.end __start

