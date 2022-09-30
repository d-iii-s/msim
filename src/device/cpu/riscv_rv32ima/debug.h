#ifndef RISCV_RV32IMA_DEBUG_H_
#define RISCV_RV32IMA_DEBUG_H_

#include <stdint.h>
#include <stdbool.h>
#include "instr.h"
#include "csr.h"
#include "cpu.h"

typedef enum {
    rv_regname_numeric,
    rv_regname_abi,
    __rv_regname_type_count
} rv_regname_type_t;

extern char** rv_regnames;
extern char** rv_csrnames;
extern char** rv_excnames;
extern char** rv_interruptnames;


extern void rv_debug_init(void);
extern bool rv_debug_change_regnames(rv_regname_type_t type);

extern void rv_reg_dump(rv_cpu_t *cpu);
extern void rv_idump(rv_cpu_t *cpu, uint32_t addr, rv_instr_t instr);
extern void rv_idump_phys(uint32_t addr, rv_instr_t instr);
extern void rv_csr_dump_all(rv_cpu_t *cpu);
extern bool rv_csr_dump(rv_cpu_t *cpu, csr_num_t csr);
extern bool rv_csr_dump_by_name(rv_cpu_t *cpu, const char* name);

#endif // RISCV_RV32IMA_DEBUG_H_