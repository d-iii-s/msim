/*
 * Check that special instruction for entering interactive mode works.
 */

.text
.set noat
.set noreorder
.ent __start
__start:
    /*
     * Enter interactive mode.
     */
	nop
	.insn
	.word 0x29
	nop

	/*
	 * Terminate.
	 */	
	.insn
	.word 0x28
	nop
.end __start

