# Shared between src/ and tests/

MSIM_SOURCES_COMMON = \
	utils.c \
	fault.c \
	text.c \
	env.c \
	cmd.c \
	main.c \
	parser.c \
	list.c \
	input.c \
	physmem.c \
	debug/debug.c \
	debug/gdb.c \
	debug/breakpoint.c \
	debug/dap.c \
	device/cpu/mips_r4000/cpu.c \
	device/cpu/mips_r4000/debug.c \
	device/cpu/riscv_rv32ima/cpu.c \
	device/cpu/riscv_rv32ima/csr.c \
	device/cpu/riscv_rv32ima/instr.c \
	device/cpu/riscv_rv32ima/tlb.c \
	device/cpu/riscv_rv32ima/mnemonics.c \
	device/cpu/riscv_rv32ima/debug.c \
	device/cpu/general_cpu.c \
	device/mem.c \
	device/ddisk.c \
	device/dr4kcpu.c \
	device/drvcpu.c  \
	device/dcycle.c \
	device/dkeyboard.c \
	device/dlcd.c \
	device/dnomem.c \
	device/dorder.c \
	device/dprinter.c \
	device/dtime.c \
	device/device.c \
	arch/win32/mmap.c \
	arch/win32/stdin.c \
	arch/win32/signal.c \
	arch/posix/stdin.c \
	arch/posix/signal.c

MSIM_SOURCES_RV64 = \
	device/cpu/riscv_rv64ima/cpu.c \
	device/cpu/riscv_rv64ima/csr.c \
	device/cpu/riscv_rv64ima/instr.c \
	device/cpu/riscv_rv64ima/tlb.c \
	device/cpu/riscv_rv64ima/debug.c \
	device/cpu/riscv_rv64ima/mnemonics.c \
	device/drv64cpu.c

ifeq ($(BUILD_RV64),yes)
MSIM_SOURCES = $(MSIM_SOURCES_COMMON) $(MSIM_SOURCES_RV64)
else
MSIM_SOURCES = $(MSIM_SOURCES_COMMON)
endif

