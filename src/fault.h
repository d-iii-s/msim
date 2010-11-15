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

#define ERR_OK      0  /**< OK */
#define ERR_IO      1  /**< I/O error occures */
#define ERR_MEM     2  /**< Not enough memory */
#define ERR_INIT    3  /**< Initial script fails */
#define ERR_PARM    4  /**< Invalid parameter */
#define ERR_INTERN  5  /**< Internal error */

extern size_t *lineno_ptr;

/** Print error message to stderr */
extern void error(const char *fmt, ...);

/** Print internal error to stderr */
extern void intr_error(const char *msg, ...);

/** Print message to stderr and exits */
extern void die(int status, const char *fmt, ...);

/** Dump error description of I/O error */
extern void io_error(const char *fname);

/** Like io_error() but call exit */
extern void io_die(int status, const char *fname);

extern void set_script(const char *sname);
extern void set_lineno(size_t no);
extern void unset_script(void);

#endif
