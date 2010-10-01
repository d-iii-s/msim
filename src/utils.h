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
#include <unistd.h>

#include "mtypes.h"

#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define safe_free(ptr) \
	{ \
		free(ptr); \
		ptr = NULL; \
	}

#define safe_malloc_t(type) (safe_malloc(sizeof(type)))

extern void *safe_malloc(const size_t size);
extern char *safe_strdup(const char *str);

extern bool prefix(const char *pref, const char *str);
extern void convert_size32_to_readable_string(char *s, uint32_t i);

extern bool addr_word_aligned(ptr_t addr);

extern bool try_open(int *fd, int flags, const char *filename);
extern bool try_close(int fd, const char *filename);
extern void try_soft_close(int fd, const char *filename);
extern bool try_lseek(int fd, off_t *offset, int whence, const char *filename);

#endif /* UTILS_H_ */
