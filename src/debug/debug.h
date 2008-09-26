/*
 * Copyright (c) 2002-2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Useful debugging features
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

#include "../cpu/instr.h"
#include "../cpu/processor.h"

extern bool dump_addr;
extern bool dump_instr;
extern bool dump_inum;

extern void reg_view(processor_t *pr);
extern void tlb_dump(processor_t *pr);
extern void cp0_dump(processor_t *pr, int reg);
extern void iview(processor_t *pr, addr_t addr, instr_info *ii, char *regch);
extern void modified_regs_dump(processor_t *pr, size_t size, char *sx);

extern void dbg_dev_dump(void);
extern void dbg_dev_stat(void);
extern void dbg_msd_dump(void);

#endif /* DEBUG_H_ */
