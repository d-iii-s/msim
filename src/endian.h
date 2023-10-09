/*
 * Copyright (c) 2001-2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Endianess conversion
 *
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

#include <inttypes.h>
#include "../config.h"

#ifdef WORDS_BIGENDIAN

#define convert_uint8_t_endian(val)  (val)

#define convert_uint16_t_endian(val) \
    ((((val) << 8) & UINT16_C(0xff00)) | (((val) >> 8) & UINT16_C(0x00ff)))

#define convert_uint32_t_endian(val) \
    ((((val) & UINT32_C(0x000000ff)) << 24) | (((val) & UINT32_C(0x0000ff00)) << 8) | \
    (((val) & UINT32_C(0x00ff0000)) >> 8) | (((val) & UINT32_C(0xff000000)) >> 24))

#define convert_uint64_t_endian(val) \
    ((((val) & UINT64_C(0x00000000000000ff)) << 56) | (((val) & UINT64_C(0x000000000000ff00)) << 40) | \
    (((val) & UINT64_C(0x0000000000ff0000)) << 24) | (((val) & UINT64_C(0x00000000ff000000)) << 8) | \
    (((val) & UINT64_C(0x000000ff00000000)) >> 8) | (((val) & UINT64_C(0x000000ff00000000)) >> 24) | \
    (((val) & UINT64_C(0x00ff000000000000)) >> 40) | (((val) & UINT64_C(0xff00000000000000)) >> 56))

#else /* WORDS_BIGENDIAN */

#define convert_uint8_t_endian(val)   (val)
#define convert_uint16_t_endian(val)  (val)
#define convert_uint32_t_endian(val)  (val)
#define convert_uint64_t_endian(val)  (val)

#endif /* WORDS_BIGENDIAN */

#endif
