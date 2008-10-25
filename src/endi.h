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

#ifndef ENDI_H_
#define ENDI_H_

#include <stdbool.h>

#include "mtypes.h"
#include "../config.h"

#ifdef WORDS_BIGENDIAN
	#define convert_uint8_t_endian(b) (b)
	#define convert_uint16_t_endian(h) \
		((((h) << 8) & 0xff00U) | (((h) >> 8) & 0x00ffU))
	#define convert_uint32_t_endian(w)	\
		((((w) & 0xffU) << 24) | (((w) & 0xff00U) << 8) | \
		(((w) & 0xff0000U) >> 8) | (((w) & 0xff000000U) >> 24))
#else
	#define convert_uint8_t_endian(b) (b)
	#define convert_uint16_t_endian(h) (h)
	#define convert_uint32_t_endian(w) (w)
#endif

#endif /* ENDI_H_ */
