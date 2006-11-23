/*
 * Copyright (c) 2001-2004 Viliam Holub
 */


#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#define TBRK	"\001"
#define TBRK_C	'\001'

void output_init( void);
void mprintf( const char *fmt, ...);
void mprintf_btag( const char *nl, const char *fmt, ...);
void mprintf_n( int n, const char *fmt, ...);
void mprintf_text( const char *in, const char *fmt, ...);
void mprintf_err( const char *fmt, ...);

#endif /* _OUTPUT_H_ */
