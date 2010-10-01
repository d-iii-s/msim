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
#include <stdio.h>

#include "mtypes.h"

#define MAX(a, b)  (((a) < (b)) ? (b) : (a))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define safe_free(ptr) \
	{ \
		free(ptr); \
		ptr = NULL; \
	}

#define safe_malloc_t(type) \
	(safe_malloc(sizeof(type)))

extern void *safe_malloc(const size_t size);
extern char *safe_strdup(const char *str);

extern bool prefix(const char *pref, const char *str);
extern char *uint32_human_readable(uint32_t i);

extern bool addr_word_aligned(ptr_t addr);

extern FILE *try_fopen(const char *path, const char *mode);
extern bool try_fclose(FILE *file, const char *path);

extern void try_soft_fclose(FILE *file, const char *path);
extern bool try_fseek(FILE *file, size_t offset, int whence, const char *path);
extern bool try_ftell(FILE *file, const char *path, size_t *pos);

#endif
