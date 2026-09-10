MIPS32_TOOLCHAIN_BIN =

MIPS32_ASFLAGS = \
	-march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 \
	-mno-abicalls -fno-pic -fno-builtin -ffreestanding \
	-nostdlib -nostdinc \
	"-Dsimulator_reg_dump=.word 0x37" \
	"-Dsimulator_halt=.word 0x28" \
	-pipe -Wall -Wextra -Werror -g3
MIPS32_CFLAGS = \
	-march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 \
	-mno-abicalls -fno-pic -fno-builtin -ffreestanding \
	-nostdlib -nostdinc \
	-Isrc/shared/include \
	-DARCH=MIPS32 -DARCH_MIPS32 \
	-pipe -Wall -Wextra -Werror \
	-O2
MIPS32_LDFLAGS = -G 0 -static -g
MIPS32_AS = $(MIPS32_TOOLCHAIN_BIN)mipsel-linux-gnu-gcc
MIPS32_CC = $(MIPS32_TOOLCHAIN_BIN)mipsel-linux-gnu-gcc
MIPS32_LD = $(MIPS32_TOOLCHAIN_BIN)mipsel-linux-gnu-ld
MIPS32_OBJCOPY = $(MIPS32_TOOLCHAIN_BIN)mipsel-linux-gnu-objcopy


RISCV32_TOOLCHAIN_BIN =

RISCV32_ASFLAGS = \
	-march=rv32g -msmall-data-limit=0 -mstrict-align \
	-fno-pic -fno-builtin -ffreestanding \
	-nostdlib -nostdinc \
	"-Dsimulator_reg_dump=.word 0x8C100073" \
	"-Dsimulator_halt=.word 0x8C000073" \
	-pipe -Wall -Wextra -Werror -g3
RISCV32_CFLAGS = \
	-march=rv32g -msmall-data-limit=0 -mstrict-align \
	-fno-pic -mno-riscv-attribute -fno-builtin -ffreestanding \
	-nostdlib -nostdinc \
	-Isrc/shared/include \
	-DARCH=RISCV32 -DARCH_RISCV32 \
	-pipe -Wall -Wextra -Werror \
	-O2
RISCV32_LDFLAGS = -G 0 -static -g
RISCV32_AS = $(RISCV32_TOOLCHAIN_BIN)riscv32-unknown-elf-gcc
RISCV32_CC = $(RISCV32_TOOLCHAIN_BIN)riscv32-unknown-elf-gcc
RISCV32_LD = $(RISCV32_TOOLCHAIN_BIN)riscv32-unknown-elf-ld
RISCV32_OBJCOPY = $(RISCV32_TOOLCHAIN_BIN)riscv32-unknown-elf-objcopy

