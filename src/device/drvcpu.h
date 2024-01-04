/*
 * Copyright (c) 2022   Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V RV32IMA device
 *
 */

#ifndef DRVCPU_H_
#define DRVCPU_H_

#include "cpu/general_cpu.h"
#include "cpu/riscv_rv32ima/cpu.h"
#include "device.h"

#define get_rv(dev) ((rv_cpu_t *) (((general_cpu_t *) (dev)->data)->data))

extern device_type_t drvcpu;

#endif // DRVCPU_H_
