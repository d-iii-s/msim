/*
 *
 * Copyright (c) 2004 Viliam Holub
 */

#ifndef _CHECK_H_
#define _CHECK_H_

/*
 * Debug flag
 * XXX move to configure
 */
#define RQ_DEBUG

/*
 * Assert routines
 */
#ifdef RQ_DEBUG
#	define REQUIRED(...)	RQ_fail( __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__, -2, #__VA_ARGS__);
#else
#	define REQUIRED( q)
#endif


#ifdef RQ_DEBUG
void RQ_fail( const char *filename, int lineno, const char *func, ...);
#endif

#endif /* _CHECK_H_ */
