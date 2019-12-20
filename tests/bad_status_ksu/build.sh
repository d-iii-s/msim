#!/bin/sh

set -x

TOOLCHAIN_DIR="$1"

"${TOOLCHAIN_DIR}mipsel-linux-gnu-gcc" \
    -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 \
    -mno-abicalls -fno-pic -fno-builtin -ffreestanding \
    -nostdlib -nostdinc \
    -pipe -Wall -Wextra -Werror \
    -g3 -c -o main.o \
    main.S \
&& "${TOOLCHAIN_DIR}mipsel-linux-gnu-ld" \
    -G 0 -static -g -T ../mips32.lds \
    -o boot.raw main.o \
&& "${TOOLCHAIN_DIR}mipsel-linux-gnu-objcopy" \
    -O binary boot.raw boot.bin
