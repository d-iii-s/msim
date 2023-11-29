/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_MEMOPS_H_
#define SUPERH_SH2E_MEMOPS_H_

#include "../../../physmem.h"

#include <stdint.h>


static inline uint8_t
sh2e_physmem_read8(
    unsigned int const cpu_id, uint32_t const addr, bool const protected
) {
    return physmem_read8(cpu_id, addr, protected);
}


static inline uint16_t
sh2e_physmem_read16(
    unsigned int const cpu_id, uint32_t const addr, bool const protected
) {
    return be16toh(physmem_read16(cpu_id, addr, protected));
}


static inline uint32_t
sh2e_physmem_read32(
    unsigned int const cpu_id, uint32_t const addr, bool const protected
) {
    return be32toh(physmem_read32(cpu_id, addr, protected));
}


#endif // SUPERH_SH2E_MEMOPS_H_
