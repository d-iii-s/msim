OUTPUT_ARCH(sh2)

SECTIONS
{
    .vectors 0x0 : { *(.vectors) }

    .main 0x400 : {
        *(__start)
        *(.text  .text.*)
        *(.data)
        *(.rodata .rodata.*)
        *(.bss)
        *(COMMON)
    }
}
