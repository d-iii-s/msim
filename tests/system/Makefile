
MIPS32_TOOLCHAIN_DIR =

MIPS32_TESTS = \
	dnomem-break \
	dnomem-halt \
	dnomem-rd \
	dnomem-warn \
	dval \
	hello \
	rd \
	xint

MIPS32_ASFLAGS = \
	-march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 \
	-mno-abicalls -fno-pic -fno-builtin -ffreestanding \
	-nostdlib -nostdinc \
	-pipe -Wall -Wextra -Werror -g3
MIPS32_LDFLAGS = -G 0 -static -g -T mips32.lds
MIPS32_AS = $(MIPS32_TOOLCHAIN_DIR)mipsel-linux-gnu-gcc
MIPS32_LD = $(MIPS32_TOOLCHAIN_DIR)mipsel-linux-gnu-ld
MIPS32_OBJCOPY = $(MIPS32_TOOLCHAIN_DIR)mipsel-linux-gnu-objcopy

MIPS32_BOOT_IMAGES = $(addprefix mips32-, $(addsuffix /boot.bin, $(MIPS32_TESTS)))

all:
	@echo "Run either make mips32 or make riscv32 to rebuild binaries."

.PHONY: all mips32 riscv32

mips32: $(MIPS32_BOOT_IMAGES)

mips32-%/boot.bin: mips32-%/boot.raw
	$(MIPS32_OBJCOPY) -O binary $< $@

mips32-%/boot.raw: mips32-%/main.o
	$(MIPS32_LD) $(MIPS32_LDFLAGS) -o $@ $<

mips32-%/main.o: mips32-%/main.S
	$(MIPS32_AS) $(MIPS32_ASFLAGS) -c -o $@ $<

