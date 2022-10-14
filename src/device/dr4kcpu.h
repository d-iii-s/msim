/*
 * Copyright (c) 2003-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS R4000 microprocessor (32 bit part) device
 *
 */

#ifndef DR4KCPU_H_
#define DR4KCPU_H_

#include "device.h"
#include "cpu/general_cpu.h"
#include "cpu/mips_r4000/cpu.h"

#define MAX_CP0_REGISTERS 32

#define get_r4k(dev) (r4k_cpu_t *)(((general_cpu_t *)(dev)->data)->data)

extern device_type_t dr4kcpu;

#endif
