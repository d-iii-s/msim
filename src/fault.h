/*
 * Copyright (c) 2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Fault handlers
 *
 */

#ifndef FAULT_H_
#define FAULT_H_

/**< Print error message to stderr */
extern void error(const char *fmt, ...);

/**< Print message to stderr and exits */
extern void die(int ex, const char *fmt, ...);

/* Dump error description of I/O error */
extern void io_error(const char *filename);

/* Like io_error() but call exit */
extern void io_die(int ex, const char *filename);

#endif /* FAULT_H_ */
