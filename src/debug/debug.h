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

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "../cpu/instr.h"
#include "../cpu/cpu.h"
#include "../device/device.h"

extern bool dump_addr;
extern bool dump_instr;
extern bool dump_inum;

extern void reg_view(cpu_t *cpu);
extern void tlb_dump(cpu_t *cpu);
extern void cp0_dump(cpu_t *cpu, int reg);
extern void iview(cpu_t *cpu, ptr_t addr, instr_info_t *ii, char *regch);
extern char *modified_regs_dump(cpu_t *cpu);

extern void dbg_print_devices(const char* header, const char* nothing_msg,
    device_filter_t filter, void (print_function) (device_s*));

extern void dbg_print_device_info(device_s *dev);
extern void dbg_print_device_statistics(device_s *dev);

#endif /* DEBUG_H_ */
