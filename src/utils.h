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

#define safe_free(ptr) \
	{ \
		free(ptr); \
		ptr = NULL; \
	}

#define safe_malloc_t(type) (safe_malloc(sizeof(type)))

extern void *safe_malloc(const size_t size);
extern char *safe_strdup(const char *str);
extern bool prefix(const char *pref, const char *str);

#endif /* UTILS_H_ */
