/*
 * dcpu.h
 * R4000 microprocessor (32bit) device
 * Copyright (c) 2003,2004 Viliam Holub
 */

#ifndef _DCPU_H_
#define _DCPU_H_

#include "device.h"
#include "processor.h"

extern device_type_s DCPU;
extern int R4000_cnt;

processor_s *cpu_find_no( int no);
void dcpu_interrupt_up( int cpuno, int no);
void dcpu_interrupt_down( int cpuno, int no);

#endif /* _DCPU_H_ */
