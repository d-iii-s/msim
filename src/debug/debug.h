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
#include "../cpu/processor.h"
#include "../device/device.h"

extern bool dump_addr;
extern bool dump_instr;
extern bool dump_inum;

extern void reg_view(processor_t *pr);
extern void tlb_dump(processor_t *pr);
extern void cp0_dump(processor_t *pr, int reg);
extern void iview(processor_t *pr, ptr_t addr, instr_info_t *ii, char *regch);
extern void modified_regs_dump(processor_t *pr, size_t size, char *sx);

extern void dbg_print_devices(const char* header, const char* nothing_msg,
    device_filter_t filter, void (print_function) (device_s*));

extern void dbg_print_device_info(device_s *dev);
extern void dbg_print_device_statistics(device_s *dev);

#endif /* DEBUG_H_ */
