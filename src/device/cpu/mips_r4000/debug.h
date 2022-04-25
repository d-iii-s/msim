#ifndef MIPS_R4000_DEBUG_H_
#define MIPS_R4000_DEBUG_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "cpu.h"
#include "../../../utils.h"

/** Debugging register names */
extern char **r4k_regname;
extern char **r4k_cp0name;
extern char **r4k_cp1name;
extern char **r4k_cp2name;
extern char **r4k_cp3name;

extern void r4k_debug_init();

extern void r4k_reg_dump(r4k_cpu_t *cpu);
extern void r4k_tlb_dump(r4k_cpu_t *cpu);
extern void r4k_cp0_dump_all(r4k_cpu_t *cpu);
extern void r4k_cp0_dump(r4k_cpu_t *cpu, unsigned int reg);

extern void r4k_idump_phys(ptr36_t addr, r4k_instr_t instr);
extern void r4k_idump(r4k_cpu_t *cpu, ptr64_t addr, r4k_instr_t instr, bool modregs);

extern char *r4k_modified_regs_dump(r4k_cpu_t *cpu);

#endif // MIPS_R4000_DEBUG_H_