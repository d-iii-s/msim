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

CCFLAGS = -ffile-prefix-map=$(SRC_BASE)= -I$(SRC_BASE)include -march=rv32g -msmall-data-limit=0 -mstrict-align -fno-pic -mno-riscv-attribute -fno-builtin -ffreestanding -nostdlib -nostdinc -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -Werror=pointer-arith -O2 -pipe -g3 -std=c11
ASFLAGS = -I$(SRC_BASE)include -march=rv32g -msmall-data-limit=0 -mstrict-align -fno-pic -mno-riscv-attribute -fno-builtin -ffreestanding -nostdlib -nostdinc -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -Werror=pointer-arith -pipe -g3 -std=c11 -D__ASM__
LDFLAGS = -G 0 -static -g

DEPEND = Makefile.depend
DEPEND_PREV = Makefile.depend.prev

### Kernel source files

# Note that KERNEL_TEST_SOURCES and KERNEL_EXTRA_CFLAGS are defined in
# config.mk as specified during configure.

KERNEL_SOURCES = \
	src/main.c \
	src/head.S \
	src/common.c

BOOT_SOURCES = \
	boot/loader.S

KERNEL_OBJECTS := $(addsuffix .o,$(basename $(KERNEL_SOURCES) $(KERNEL_TEST_SOURCES)))
KERNEL_LINKER_SCRIPT = $(SRC_BASE)kernel.lds
KERNEL_RAW = kernel.raw
KERNEL_MAP = kernel.map
KERNEL_BINARY = kernel.bin
KERNEL_DISASM = kernel.disasm

BOOT_OBJECTS := $(addsuffix .o,$(basename $(BOOT_SOURCES)))
BOOT_LINKER_SCRIPT = $(SRC_BASE)loader.lds
BOOT_RAW = loader.raw
BOOT_MAP = loader.map
BOOT_BINARY = loader.bin
BOOT_DISASM = loader.disasm

DEPS := $(addsuffix .dep,$(basename $(KERNEL_SOURCES) $(KERNEL_TEST_SOURCES) $(BOOT_SOURCES)))

.PHONY: .FORCE all clean distclean dir-tree

all: $(BOOT_BINARY) $(BOOT_DISASM) $(KERNEL_BINARY) $(KERNEL_DISASM)

dir-tree:
	for i in $(KERNEL_OBJECTS) $(BOOT_OBJECTS); do mkdir -p `dirname $$i`; done

distclean: clean
	rm -f $(KERNEL_BINARY) $(BOOT_BINARY)

clean:
	rm -f $(KERNEL_DISASM) $(KERNEL_MAP) $(KERNEL_RAW) $(BOOT_DISASM) $(BOOT_MAP) $(BOOT_RAW) $(GENERATOR_ASM) $(OFFSET_HEADER) $(DEPEND) $(DEPEND_PREV)
	find . -\( -name '*.o' -o -name '*.dep' -\) -exec rm -f \{\} \;

-include $(DEPS)

%.disasm: %.raw
	$(OBJDUMP) -d -S $< > $@

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

$(BOOT_RAW): $(BOOT_LINKER_SCRIPT) $(BOOT_OBJECTS)
	$(LD) $(LDFLAGS) -T $(BOOT_LINKER_SCRIPT) -Map $(BOOT_MAP) -o $@ $(BOOT_OBJECTS)

$(KERNEL_RAW): $(KERNEL_LINKER_SCRIPT) $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKER_SCRIPT) -Map $(KERNEL_MAP) -o $@ $(KERNEL_OBJECTS)

### Default patterns

%.o: %.c | dir-tree
	$(CC) $(CCFLAGS) $(KERNEL_EXTRA_CFLAGS) -MD -MF $*.dep -c -o $@ $<

%.o: %.S | dir-tree
	$(CC) $(ASFLAGS) $(KERNEL_EXTRA_ASFLAGS) -c -o $@ $<
