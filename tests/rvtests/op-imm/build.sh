#!/bin/bash
riscv32-unknown-elf-gcc -march=rv32im -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -c -o main.raw main.S
riscv32-unknown-elf-objdump -d -C -S main.raw > main.dis
riscv32-unknown-elf-objcopy -O binary main.raw main.bin
