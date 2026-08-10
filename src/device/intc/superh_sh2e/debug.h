/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_INTC_DEBUG_H_
#define SUPERH_SH2E_INTC_DEBUG_H_

#include "intc.h"

extern void sh2e_intc_dump_regs(sh2e_intc_t const *intc);

extern void sh2e_intc_dump_configuration(sh2e_intc_t *intc);

#endif // SUPERH_SH2E_INTC_DEBUG_H_
