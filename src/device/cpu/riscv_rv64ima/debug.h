/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Debugging utilities
 *
 */

#ifndef RISCV_RV64IMA_DEBUG_H_
#define RISCV_RV64IMA_DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"
#include "csr.h"

/** Generics */
#include "../riscv_rv_ima/instr.h"

/** The types of the general register names*/
typedef enum {
    rv64_regname_numeric,
    rv64_regname_abi,
    __rv64_regname_type_count
} rv64_regname_type_t;

extern char **rv64_regnames; /** The currently selected names of registers */
extern char **rv64_csrnames; /** The names of CSRs */
extern char **rv64_excnames; /** The names of exceptions */
extern char **rv64_interruptnames; /** The names of interrupts */

extern void rv64_debug_init(void);
extern bool rv64_debug_change_regnames(rv64_regname_type_t type);

extern void rv64_reg_dump(rv64_cpu_t *cpu);
extern void rv64_idump(rv64_cpu_t *cpu, uint32_t addr, rv_instr_t instr);
extern void rv64_idump_phys(uint32_t addr, rv_instr_t instr);
extern void rv64_csr_dump_all(rv64_cpu_t *cpu);
extern void rv64_csr_dump_mmode(rv64_cpu_t *cpu);
extern void rv64_csr_dump_smode(rv64_cpu_t *cpu);
extern void rv64_csr_dump_counters(rv64_cpu_t *cpu);
extern void rv64_csr_dump_reduced(rv64_cpu_t *cpu);
extern bool rv64_csr_dump(rv64_cpu_t *cpu, csr_num_t csr);
extern bool rv64_csr_dump_by_name(rv64_cpu_t *cpu, const char *name);
extern bool rv64_csr_dump_command(rv64_cpu_t *cpu, const char *command);
extern bool rv64_translate_dump(rv64_cpu_t *cpu, uint64_t addr);
extern bool rv64_translate_sv39_dump(rv64_cpu_t *cpu, ptr36_t root_pagetable_phys, uint64_t addr);
extern bool rv64_pagetable_dump(rv64_cpu_t *cpu, bool verbose);
extern bool rv64_pagetable_dump_from_phys(rv64_cpu_t *cpu, ptr36_t root_pagetable_phys, bool verbose);

#endif // RISCV_RV64IMA_DEBUG_H_
