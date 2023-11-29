/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_DEBUG_H_
#define SUPERH_SH2E_DEBUG_H_

#include "cpu.h"

#include "../../../utils.h"

#include <stdbool.h>
#include <stdint.h>


/** The types of the register naming styles. */

typedef enum {
    SH2E_REGNAME_STYLE_NUMERIC,
    SH2E_REGNAME_STYLE_ABI,
    __SH2E_REGNAME_STYLE_COUNT,
} sh2e_regname_style_t;

extern void sh2e_debug_init(void);
extern bool sh2e_debug_set_regname_style(sh2e_regname_style_t style);

extern void sh2e_cpu_dump_cpu_regs(sh2e_cpu_t const * cpu);
extern void sh2e_cpu_dump_fpu_regs(sh2e_cpu_t const * cpu);

extern void sh2e_cpu_dump_insn(sh2e_cpu_t const * cpu, uint32_t addr, sh2e_insn_t insn);
extern void sh2e_cpu_dump_code(sh2e_cpu_t const * cpu, uint32_t start_addr, unsigned int count);
extern void sh2e_cpu_dump_data(sh2e_cpu_t const * cpu, uint32_t start_addr, unsigned int count);

extern void sh2e_dump_insn_phys(ptr36_t addr);


#endif // SUPERH_SH2E_DEBUG_H_
