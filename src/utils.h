/*
 * Copyright (c) 2004-2007 Viliam Holub
 * Copyright (c) 2008-2011 Martin Decky
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
#include <stdint.h>
#include <stdio.h>
#include "main.h"

#define MAX(a, b)  (((a) < (b)) ? (b) : (a))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define ALIGN_DOWN(addr, align) \
	((addr) & ~((align) - 1))

#define ALIGN_UP(addr, align) \
	(((addr) + ((align) - 1)) & ~((align) - 1))

#define IS_ALIGNED(addr, align) \
	((addr & (align - 1)) == 0)

#define BIT_MASK(start, end) \
	(((UINT64_C(1) << (end - start)) - 1) << start)

#define EXTRACT_BITS(val, start, end) \
	((val >> start) & ( (UINT64_C(1) << (end - start)) - 1))

#define WRITE_BITS(target, val, start, end) \
	((target & ~BIT_MASK(start, end)) | ((val << start) & BIT_MASK(start, end)))

#define AREAS_OVERLAP(base1, size1, base2, size2) \
	(((base1) >= (base2) && (base1) < (base2) + (size2)) || ((base2) >= (base1) && (base2) < (base1) + (size1)))

#define safe_free(ptr) \
	{ \
		if (ptr != NULL) { \
			free(ptr); \
			ptr = NULL; \
		} \
	}

#define safe_malloc_t(type) \
	((type *) safe_malloc(sizeof(type)))

typedef struct {
	char *str;
	size_t size;
	size_t pos;
} string_t;

extern void *safe_malloc(const size_t size);
extern char *safe_strdup(const char *str);

extern void string_init(string_t *str);
extern void string_clear(string_t *str);
extern bool string_is_empty(string_t *str);
extern void string_push(string_t *str, char c);
extern void string_append(string_t *str, const char *val);
extern void string_vprintf(string_t *str, const char *fmt, va_list va);
extern void string_printf(string_t *str, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
extern void string_fread(string_t *str, FILE *file);
extern void string_done(string_t *str);

extern bool prefix(const char *pref, const char *str);
extern char *uint64_human_readable(uint64_t i);

extern bool ptr36_dword_aligned(ptr36_t addr);
extern bool ptr36_frame_aligned(ptr36_t addr);

extern bool virt_range(uint64_t addr);
extern bool phys_range(uint64_t addr);

extern FILE *try_fopen(const char *path, const char *mode);
extern bool check_isdir(FILE *file);
extern bool try_fseek(FILE *file, size_t offset, int whence, const char *path);
extern bool try_ftell(FILE *file, const char *path, size_t *pos);
extern void safe_fclose(FILE *file, const char *path);

extern void try_munmap(void *ptr, size_t size);

extern uint64_t current_timestamp(void);

#endif
