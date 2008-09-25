/*
 * Copyright (c) 2003-2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Output routines
 *
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#define TBRK   "\001"
#define TBRK_C '\001'

extern void output_init(void);
extern void mprintf(const char *fmt, ...);
extern void mprintf_btag(const char *nl, const char *fmt, ...);
extern void mprintf_n(int n, const char *fmt, ...);
extern void mprintf_text(const char *in, const char *fmt, ...);
extern void mprintf_err(const char *fmt, ...);

#endif /* OUTPUT_H_ */
