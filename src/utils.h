/*
 * Copyright (c) 2004-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Small useful routines
 *
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdbool.h>

#define XFREE(x) { \
	free(x); \
	x = NULL; \
}

#define XXMALLOC(type) (xmalloc(sizeof(type)))
#define XCMALLOC(type, count) (xmalloc(sizeof(type) * count))

extern void *xmalloc(size_t size);
extern char *xstrdup(const char *str);
extern bool prefix(const char *pref, const char *str);

#endif /* UTILS_H_ */
