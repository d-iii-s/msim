CC := sh-unknown-elf-gcc
CFLAGS := -m2e -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -g
LD := sh-unknown-elf-ld
OBJCOPY := sh-unknown-elf-objcopy
OBJDUMP := sh-unknown-elf-objdump

MAIN_SRC := main.c head.S
MAIN_OBJ := $(addsuffix .o,$(basename $(MAIN_SRC)))
MAIN_RAW := main.raw
MAIN_BIN := main.bin
MAIN_DISASM := main.dis
MAIN_LINKER_SCRIPT := main.lds
MAIN_LINKER_MAP := main.map

.PHONY: all clean

all: $(MAIN_BIN) $(MAIN_DISASM) $(MAIN_RAW)

clean:
	rm -f $(MAIN_DISASM) $(MAIN_LINKER_MAP) $(MAIN_RAW) $(MAIN_OBJ)

$(MAIN_RAW): $(MAIN_OBJ) $(MAIN_LINKER_SCRIPT)
	$(LD) -T $(MAIN_LINKER_SCRIPT) -Map $(MAIN_LINKER_MAP) -o $@ $(MAIN_OBJ)

%.dis: %.raw
	$(OBJDUMP) -d -S $< > $@

%.bin: %.raw
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<