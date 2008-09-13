/*
 * endi.h
 * checking endian
 * Copyright (c) 2001-2005 Viliam Holub
 */


#ifndef _ENDI_H_
#define _ENDI_H_

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdbool.h>

#include "mtypes.h"

extern bool little_endian;

void checkendian( void);
bool test_type( void);

#ifdef WORDS_BIGENDIAN
# define convert_uint8_t_endian( b)	(b)
# define convert_uint16_t_endian( h)	\
		( (((h) << 8) & 0xff00U) | (((h) >> 8) & 0x00ffU) )
# define convert_uint32_t_endian( w)	\
		( (((w)&0xffU)<<24) | (((w)&0xff00U)<<8) | \
		(((w)&0xff0000U)>>8) | (((w)&0xff000000U)>>24) );
#else
# define convert_uint8_t_endian( b)	(b)
# define convert_uint16_t_endian( h)	(h)
# define convert_uint32_t_endian( w)	(w)
#endif

#endif /* _ENDI_H_ */
