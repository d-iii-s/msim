/*
 * endi.h
 * checking endian
 * Copyright (c) 2001, 2002, 2003 Viliam Holub
 */


#ifndef _ENDI_H_
#define _ENDI_H_

#include "../config.h"

#include "mtypes.h"

extern bool little_endian;

void checkendian( void);
bool test_type( void);

#ifdef WORDS_BIGENDIAN
# define convert_uint8_t_endian( b)	(b)
# define convert_uint16_t_endian( h)	\
		( (((w) << 8) & 0xff00U) | (((w) >> 8) & 0x00ffU) )
# define convert_uint32_t_endian( d)	\
		( (((d)&0xffU)<<24) | (((d)&0xff00U)<<8) | \
	 	(((d)&0xff0000U)>>8) | (((d)&0xff000000U)>>24) );
#else
# define convert_uint8_t_endian( b)	(b)
# define convert_uint16_t_endian( h)	(h)
# define convert_uint32_t_endian( w)	(w)
#endif

#endif /* _ENDI_H_ */
