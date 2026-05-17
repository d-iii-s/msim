/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E Compare Match Timer device.
 *
 */

#ifndef DSH2ECMT_H_
#define DSH2ECMT_H_

#include "device.h"

extern device_type_t const dsh2ecmt;

extern void cmt_reset(void);

extern void cmt_cpu_cycles_update(unsigned int cycles);

#endif
