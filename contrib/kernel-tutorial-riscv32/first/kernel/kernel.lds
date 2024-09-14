/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2019 Charles University */

OUTPUT_ARCH(riscv)

SECTIONS {
	PROVIDE (_kernel_start = 0x80000000);

	/* Kernel image assembled from text and data sections */

	.kernel 0x80000000 : {
		/* Empty section at the start that is used for stack space */
		*(.head)
		*(.handlers)
		/* The usual sections */
		*(.kernel_padding_head)
		*(.text .text.*)
		_kernel_code_end = .;
		*(.data)
		*(.rodata .rodata.*)
		*(.bss .bss.*)
		*(COMMON)
		*(.kernel_padding_tail)
		_kernel_end = .;
	}

	/* Debugging information copied from default linker script */

	.stab            0 : { *(.stab) }
	.stabstr         0 : { *(.stabstr) }
	.stab.excl       0 : { *(.stab.excl) }
	.stab.exclstr    0 : { *(.stab.exclstr) }
	.stab.index      0 : { *(.stab.index) }
	.stab.indexstr   0 : { *(.stab.indexstr) }
	.comment         0 : { *(.comment) }
	.debug           0 : { *(.debug) }
	.line            0 : { *(.line) }
	.debug_srcinfo   0 : { *(.debug_srcinfo) }
	.debug_sfnames   0 : { *(.debug_sfnames) }
	.debug_aranges   0 : { *(.debug_aranges) }
	.debug_pubnames  0 : { *(.debug_pubnames) }
	.debug_info      0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev    0 : { *(.debug_abbrev) }
	.debug_line      0 : { *(.debug_line) }
	.debug_frame     0 : { *(.debug_frame) }
	.debug_str       0 : { *(.debug_str) }
	.debug_loc       0 : { *(.debug_loc) }
	.debug_macinfo   0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.debug_pubtypes  0 : { *(.debug_pubtypes) }
	.debug_ranges    0 : { *(.debug_ranges) }
	.gnu.attributes  0 : { KEEP (*(.gnu.attributes)) }
	.riscv.attributes 0 : { *(.riscv.attributes)}

	/*
	 * Discarded sections
	 *
	 * Some sections need to be explicitly discarded as a
	 * workaround for objcopy errors. These sections are
	 * not needed in the resulting binary or even make no
	 * sense in this context anyway.
	 */

	/DISCARD/ : {
		*(.eh_frame)
		*(.reginfo)
	}
}
