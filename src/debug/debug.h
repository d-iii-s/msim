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
#include "../device/cpu/mips_r4000/cpu.h"
#include "../device/cpu/riscv_rv32ima/cpu.h"
#include "../device/device.h"
#include "../utils.h"

extern void dbg_print_devices(device_filter_t filter);
extern void dbg_print_devices_stat(device_filter_t filter);

extern void dbg_print_device_info(device_t *dev);
extern void dbg_print_device_stat(device_t *dev);

#endif
