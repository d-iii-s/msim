# SPDX-License-Identifier: Apache-2.0
# Copyright 2019 Charles University

### Compiler, assembler and linker options
#
# All the options are described in detail in the GCC
# and binutils documentation. A brief description of
# some of the specific ones:
#
# -O2                         ... basic optimizations
# -ffreestanding              ... a compilation without standard library and main()
# -nostdlib                   ... do not look for standard library in system directories
# -nostdinc                   ... do not look for standard header files in system directories
# -fno-pic                    ... do not generate position independent code using $gp
# -fno-builtin                ... do not recognize built-in functions without __builtin_ prefix
# -mstrict-align              ... force strict alignment, because MSIM requires it
# -mno-riscv-attribute        ... do not emit RISC-V attibute to record extra information into ELF objects
# -msmall-data-limit=0        ... do not put small static data into a special section
# -march=rv32g                ... enable only basic extensions
#


-include ../config.mk
-include Makefile.depend

CCFLAGS = -O2 -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -mno-riscv-attribute -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -march=rv32g
ASFLAGS = -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -mno-riscv-attribute -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -march=rv32g
LDFLAGS = -G 0 -static -g

LOADER_LINKER_SCRIPT = loader.lds
KERNEL_LINKER_SCRIPT = kernel.lds

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

loader.raw: $(LOADER_LINKER_SCRIPT) boot/loader.o
	$(LD) $(LDFLAGS) -T $(LOADER_LINKER_SCRIPT) -Map loader.map -o $@ boot/loader.o

kernel.raw: $(KERNEL_LINKER_SCRIPT) src/main.o src/head.o
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKER_SCRIPT) -Map kernel.map -o $@ src/main.o src/head.o

### Default patterns

%.o: %.c
	$(CC) $(CCFLAGS) $(KERNEL_EXTRA_CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<
