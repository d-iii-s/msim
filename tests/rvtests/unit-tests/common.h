#ifndef _RVTEST_COMMON_H

#ifndef ARCH
#define ARCH 64
#endif

#if ARCH == 32
#define rv_cpu rv32_cpu
#define rv_cpu_t rv32_cpu_t
#define rv_cpu_init rv32_cpu_init
#define rv_convert_addr rv32_convert_addr

#define rv_instr_decode rv32_instr_decode
#define rv_sfence_instr _rv32_sfence_instr

#define rv_csr_set_asid_len rv32_csr_set_asid_len

#include "../../../src/device/cpu/riscv_rv32ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv32ima/csr.h"

// Memory helpers
#include "../../../src/device/cpu/riscv_rv_ima/memory.c"

// Instruction implementations
#include "../../../src/device/cpu/riscv_rv32ima/instr.c"
#include "../../../src/device/cpu/riscv_rv_ima/instr.h"

#elif ARCH == 64

#define rv_cpu rv64_cpu
#define rv_cpu_t rv64_cpu_t
#define rv_cpu_init rv64_cpu_init
#define rv_convert_addr rv64_convert_addr

#define rv_instr_decode rv64_instr_decode
#define rv_sfence_instr _rv64_sfence_instr

#define rv_csr_set_asid_len rv64_csr_set_asid_len

#include "../../../src/device/cpu/riscv_rv64ima/cpu.h"
#include "../../../src/device/cpu/riscv_rv64ima/csr.h"

// Memory helpers
#include "../../../src/device/cpu/riscv_rv_ima/memory.c"

// Instruction implementations
#include "../../../src/device/cpu/riscv_rv64ima/instr.c"
#include "../../../src/device/cpu/riscv_rv_ima/instr.h"

#else
#error "Unsupported architecture for tests"
#endif

#define _RVTEST_COMMON_H
#endif // _RVTEST_COMMON_H
