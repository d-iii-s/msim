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

extern device_type_s dcpu;
extern const char id_dcpu[];

extern processor_t *dcpu_find_no(unsigned int no);
extern void dcpu_interrupt_up(unsigned int cpuno, unsigned int no);
extern void dcpu_interrupt_down(unsigned int cpuno, unsigned int no);

#endif
