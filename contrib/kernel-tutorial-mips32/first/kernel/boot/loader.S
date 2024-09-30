// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * System bootstrap code.
 *
 * When R4000 is started, it starts executing code at virtual address
 * 0xBFC00000 (maps to physical address 0x1FC00000).
 * Note that this code is in KSEG1 which allows the kernel to prepare
 * address translation etc. before it is needed.
 *
 * This code only jumps to kernel located at physical address 0x400
 * where rest of the code is stored.
 * Note that kernel code can actually be stored here but it is much simpler
 * to jump to fixed address lower in the memory and not worry whether we
 * do not use memory outside of KSEG1.
 */

.text

/*
 * We know what whare are doing here so do not let
 * assembler change our code in any way.
 */
.set noat
.set noreorder


.ent __start

__start:
    /*
     * We only jump to the real kernel code.
     *
     * Note that LA is a preprocessor instruction as R4000 does not
     * have a separate instruction for loading 32bit constant.
     * Thus, the constant is loaded in two 16bit parts that are merged
     * together.
     *
     * Also note that we explicitly add NOP to the branch delay slot
     * to prevent any unexpected behaviour.
	 */

	la $ra, 0x80000400
	j $ra
	nop

.end __start
