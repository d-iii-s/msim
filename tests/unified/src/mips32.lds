OUTPUT_ARCH(mips)

SECTIONS {
	.kernel 0x80000000 : {
		*(.excvec)
		*(.text .text.*)
		*(.data)
		*(.rodata .rodata.*)
		*(.bss .bss.*)
		*(COMMON)
	}
	/DISCARD/ : {
		*(.eh_frame)
		*(.reginfo)
		*(.MIPS.abiflags)
	}
}
