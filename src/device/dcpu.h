/*
 * Copyright (c) 2003-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  R4000 microprocessor (32bit) device
 *
 */

#ifndef DCPU_H_
#define DCPU_H_

#include "device.h"
#include "../cpu/processor.h"

extern device_type_s DCPU;
extern int R4000_cnt;

extern processor_s *cpu_find_no(int no);
extern void dcpu_interrupt_up(int cpuno, int no);
extern void dcpu_interrupt_down(int cpuno, int no);

#endif /* DCPU_H_ */
