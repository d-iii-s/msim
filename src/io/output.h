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

extern void output_init(void);
extern void mprintf(const char *fmt, ...);
extern void mprintf_err(const char *fmt, ...);

#endif /* OUTPUT_H_ */
