# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 Charles University

### Compiler, assembler and linker options
#
# All the options are described in detail in the GCC
# and binutils documentation. A brief description of
# some of the specific ones:
#
# -O2                         ... basic optimizations
# -march=r4000                ... generate code for MIPS R4000
# -mabi=32                    ... use standard 32 bit ABI
# -mgp32                      ... general purpose registers are 32 bit
# -msoft-float                ... do not generate floating point instructions
# -mlong32                    ... ints, long and pointers are 32 bit
# -G 0                        ... threshold for putting objects into small data/bss sections
# -mno-abicalls               ... do not generate SVR4-style position independent code
# -fno-pic                    ... do not generate position independent code using $gp
# -fno-builtin                ... do not recognize built-in functions without __builtin_ prefix
# -ffreestanding              ... a compilation without standard library and main()
# -nostdlib                   ... do not look for standard library in system directories
# -nostdinc                   ... do not look for standard header files in system directories
#

-include ../config.mk

CCFLAGS = -O2 -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11
ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__
LDFLAGS = -G 0 -static -g

LINKER_SCRIPT = kernel.lds


.PHONY: .FORCE all clean distclean

all: loader.bin loader.disasm kernel.bin kernel.disasm

distclean: clean
	rm -f kernel.bin loader.bin

clean:
	rm -f *.map *.disasm *.raw
	find . -\( -name '*.o' -o -name '*.dep' -\) -exec rm -f \{\} \;

%.disasm: %.raw
	$(OBJDUMP) -d $< > $@

### Binary images
#
# The binary files are made in two stages. First, ELF files are
# created from the object files. Next, the binary files are
# created from the ELF files. The reasons for this are:
#
#  - The ELF files contain debugging information.
#
#  - Linking directly to a binary format does
#    not produce errors on missing symbols.
#

%.bin: %.raw
	$(OBJCOPY) -O binary $< $@

loader.raw: $(LINKER_SCRIPT) boot/loader.o
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -Map loader.map -o $@ boot/loader.o

kernel.raw: $(LINKER_SCRIPT) src/main.o src/head.o
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -Map kernel.map -o $@ src/main.o src/head.o

### Default patterns

%.o: %.c
	$(CC) $(CCFLAGS) $(KERNEL_EXTRA_CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<
