/*
 * Small useful routines
 *
 * Copyright (c) 2004 Viliam Holub
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdbool.h>

#define XFREE( x)	{ free( x); x=NULL; }

void *xmalloc( size_t s);
char *xstrdup( const char *s);
bool prefix( const char *pref, const char *str);

#endif /* _UTILS_H_ */
