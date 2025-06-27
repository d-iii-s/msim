/*
 * Copyright (c) 2022   Jan Papesch
 * Copyright (c) 2025   Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V RV64IMA device
 *
 */

#ifndef DRV64CPU_H_
#define DRV64CPU_H_

#include "cpu/general_cpu.h"
#include "cpu/riscv_rv64ima/cpu.h"
#include "device.h"

#define get_rv64(dev) ((rv64_cpu_t *) (((general_cpu_t *) (dev)->data)->data))

extern device_type_t drv64cpu;

#endif // DRV64CPU_H_
