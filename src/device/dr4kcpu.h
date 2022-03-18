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

#ifndef DCPU_H_
#define DCPU_H_

#include "device.h"
#include "cpu/mips_r4000/cpu.h"

#define MAX_CP0_REGISTERS 32

extern device_type_t dr4kcpu;

// todo: remove
extern r4k_cpu_t *dcpu_find_no(unsigned int no);

extern void dcpu_interrupt_up(unsigned int cpuno, unsigned int no);
extern void dcpu_interrupt_down(unsigned int cpuno, unsigned int no);

#endif
