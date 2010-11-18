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

#include "../config.h"

#ifdef WORDS_BIGENDIAN

#define convert_uint8_t_endian(val) (val)

#define convert_uint16_t_endian(val) \
	((((val) << 8) & 0xff00U) | (((val) >> 8) & 0x00ffU))

#define convert_uint32_t_endian(val) \
	((((val) & 0xffU) << 24) | (((val) & 0xff00U) << 8) | \
	(((val) & 0xff0000U) >> 8) | (((val) & 0xff000000U) >> 24))

#define convert_uint64_t_endian(val) \
	((((val) & 0xffULL) << 56) | (((val) & 0xff00ULL) << 40) | \
	(((val) & 0xff0000ULL) << 24) | (((val) & 0xff000000ULL) << 8) | \
	(((val) & 0xff00000000ULL) >> 8) | (((val) & 0xff00000000ULL) >> 24) | \
	(((val) & 0xff000000000000ULL) >> 40) | (((val) & 0xff000000000000ULL) >> 56))

#else /* WORDS_BIGENDIAN */

#define convert_uint8_t_endian(val)  (val)
#define convert_uint16_t_endian(val) (val)
#define convert_uint32_t_endian(val) (val)
#define convert_uint64_t_endian(val) (val)

#endif /* WORDS_BIGENDIAN */

#endif
