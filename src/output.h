/*
 * Copyright (c) 2001-2004 Viliam Holub
 */


#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#define TBRK	"\001"
#define TBRK_C	'\001'

void output_init( void);
void dprintf( const char *fmt, ...);
void dprintf_btag( const char *nl, const char *fmt, ...);
void dprintf_n( int n, const char *fmt, ...);
void dprintf_text( const char *in, const char *fmt, ...);
void dprintf_err( const char *fmt, ...);

#endif /* _OUTPUT_H_ */
