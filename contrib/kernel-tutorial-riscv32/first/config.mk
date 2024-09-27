# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 Charles University

# Guess where the toolchain is installed.
# We search /opt/mffd3s where our packages (please, see our tutorial)
# are installed and /usr where some distributions prefer to install cross
# compilation tools. The last branch that searches $HOME/.local is expected
# to be overriden if you use a different location for the toolchain.
ifneq ("$(wildcard /opt/mffd3s/riscv32/bin/riscv32-unknown-elf-gcc)", "")
	TOOLCHAIN_DIR = /opt/mffd3s/riscv32
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
