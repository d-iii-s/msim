/*
 * Debugging features
 * Copyright (c) Viliam Holub
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdbool.h>
#include <stdint.h>

extern bool dump_addr;
extern bool dump_instr;
extern bool dump_inum;

void reg_view( void);
void tlb_dump( void);
void CP0Dump( int reg);
void iview( uint32_t addr, TInstrInfo *ii, bool procdep, char *regch);
void modified_regs_dump( size_t siz, char *sx);

void dbg_dev_dump( void);
void dbg_dev_stat( void);
void dbg_msd_dump( void);

#endif /* _DEBUG_H_ */
