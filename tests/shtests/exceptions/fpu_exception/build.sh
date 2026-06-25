#!/bin/bash
sh-unknown-elf-gcc -m2e -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -c -o main.raw main.S
sh-unknown-elf-objdump -d -C -S main.raw > main.dis
sh-unknown-elf-objcopy -O binary main.raw main.bin

sh-unknown-elf-gcc -m2e -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -c -o handler.raw handler.S
sh-unknown-elf-objcopy -O binary handler.raw handler.bin
