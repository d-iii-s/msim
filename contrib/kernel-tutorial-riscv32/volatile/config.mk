# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 Charles University

# Guess where the toolchain is installed.
# Override the last branch if you are not in Rotunda
# or you have not used our cross-compiler build script.
ifneq ("$(wildcard /opt/mffd3s/bin/riscv32-unknown-elf-gcc)", "")
	TOOLCHAIN_DIR = /opt/mffd3s
	TARGET = riscv32-unknown-elf
else
	ifneq ("$(wildcard /usr/bin/riscv32-unknown-elf-gcc)", "")
		TOOLCHAIN_DIR = /usr
		TARGET = riscv32-unknown-elf
	else
		# Override here
		TOOLCHAIN_DIR = $(HOME)/.local
		TARGET = riscv32-unknown-elf
	endif
endif


CC = $(TOOLCHAIN_DIR)/bin/$(TARGET)-gcc
LD = $(TOOLCHAIN_DIR)/bin/$(TARGET)-ld
OBJCOPY = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objcopy
OBJDUMP = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objdump
