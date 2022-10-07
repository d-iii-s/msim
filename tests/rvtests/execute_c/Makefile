CC := riscv32-unknown-elf-gcc
CFLAGS := -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -g
LD := riscv32-unknown-elf-ld
OBJCOPY := riscv32-unknown-elf-objcopy
OBJDUMP := riscv32-unknown-elf-objdump

MAIN_SRC := main.c head.S
MAIN_OBJ := $(addsuffix .o,$(basename $(MAIN_SRC)))
MAIN_RAW := main.raw
MAIN_BIN := main.bin
MAIN_DISASM := main.disasm
MAIN_LINKER_SCRIPT := main.lds
MAIN_LINKER_MAP := main.map

LOADER_SRC := loader.S
LOADER_OBJ := $(addsuffix .o,$(basename $(LOADER_SRC)))
LOADER_RAW := loader.raw
LOADER_BIN := loader.bin
LOADER_DISASM := loader.disasm
LOADER_LINKER_SCRIPT := loader.lds
LOADER_LINKER_MAP := loader.map

.PHONY: all clean

all: $(MAIN_BIN) $(LOADER_BIN) $(MAIN_DISASM) $(LOADER_DISASM)

clean:
	rm -f $(MAIN_DISASM) $(MAIN_LINKER_MAP) $(MAIN_RAW) $(MAIN_OBJ) $(LOADER_DISASM) $(LOADER_LINKER_MAP) $(LOADER_RAW) $(LOADER_OBJ)



$(MAIN_RAW): $(MAIN_OBJ) $(MAIN_LINKER_SCRIPT)
	$(LD) -T $(MAIN_LINKER_SCRIPT) -Map $(MAIN_LINKER_MAP) -o $@ $(MAIN_OBJ)

$(LOADER_RAW): $(LOADER_OBJ) $(LOADER_LINKER_SCRIPT)
	$(LD) -T $(LOADER_LINKER_SCRIPT) -Map $(LOADER_LINKER_MAP) -o $@ $(LOADER_OBJ)

%.disasm: %.raw
	$(OBJDUMP) -d -S $< > $@

%.bin: %.raw
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<
