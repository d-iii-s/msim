/*
 * fault.h
 * fault handlers
 * Copyright (c) 2003 Viliam Holub
 */

#ifndef _FAULT_H_
#define _FAULT_H_

/* prints error message to stderr */
void error( const char *fmt, ...);

/* prints message to stderr and exits */
void die( int ex, const char *fmt, ...);

/* dump error description of io error */
void io_error( const char *filename);

/* like error but call exit */
void io_die( int n, const char *filename);

#endif /* _FAULT_H_ */
