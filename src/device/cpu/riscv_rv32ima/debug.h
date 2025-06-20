/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Debugging utilities
 *
 */

#ifndef RISCV_RV32IMA_DEBUG_H_
#define RISCV_RV32IMA_DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"
#include "csr.h"

/** Generics */
#include "../riscv_rv_ima/instr.h"

/** The types of the general register names*/
typedef enum {
    rv_regname_numeric,
    rv_regname_abi,
    __rv_regname_type_count
} rv_regname_type_t;

extern char **rv_regnames; /** The currently selected names of registers */
extern char **rv_csrnames; /** The names of CSRs */
extern char **rv_excnames; /** The names of exceptions */
extern char **rv_interruptnames; /** The names of interrupts */

extern void rv32_debug_init(void);
extern bool rv32_debug_change_regnames(rv_regname_type_t type);

extern void rv32_reg_dump(rv_cpu_t *cpu);
extern void rv32_idump(rv32_cpu_t *cpu, uint32_t addr, rv_instr_t instr);
extern void rv32_idump_phys(uint32_t addr, rv_instr_t instr);
extern void rv32_csr_dump_all(rv_cpu_t *cpu);
extern void rv32_csr_dump_mmode(rv_cpu_t *cpu);
extern void rv32_csr_dump_smode(rv_cpu_t *cpu);
extern void rv32_csr_dump_counters(rv_cpu_t *cpu);
extern void rv32_csr_dump_reduced(rv_cpu_t *cpu);
extern bool rv32_csr_dump(rv_cpu_t *cpu, csr_num_t csr);
extern bool rv32_csr_dump_by_name(rv_cpu_t *cpu, const char *name);
extern bool rv32_csr_dump_command(rv_cpu_t *cpu, const char *command);
extern bool rv32_translate_dump(rv_cpu_t *cpu, uint32_t addr);
extern bool rv32_translate_sv32_dump(rv_cpu_t *cpu, ptr36_t root_pagetable_phys, uint32_t addr);
extern bool rv32_pagetable_dump(rv_cpu_t *cpu, bool verbose);
extern bool rv32_pagetable_dump_from_phys(rv_cpu_t *cpu, ptr36_t root_pagetable_phys, bool verbose);

#endif // RISCV_RV32IMA_DEBUG_H_
