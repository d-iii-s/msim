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

#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "device/cpu/mips_r4000/cpu.h"
#include "device/cpu/riscv_rv32ima/cpu.h"
#include "arch/mmap.h"
#include "text.h"
#include "utils.h"
#include "fault.h"
#include "assert.h"
#include "main.h"

#define STRING_GRANULARITY  128
#define STRING_BUFFER       4096

static void safe_realloc(char **ptr, size_t *size, size_t pos,
    size_t granularity)
{
	ASSERT(ptr != NULL);
	ASSERT(*ptr != NULL);
	ASSERT(size != NULL);
	
	size_t sz = ALIGN_UP(MAX(*size, pos + 1), granularity);
	
	if (sz != *size) {
		*size = sz;
		*ptr = (char *) realloc(*ptr, sz);
		
		if (*ptr == NULL)
			die(ERR_MEM, "Not enough memory");
	}
}

void string_init(string_t *str)
{
	ASSERT(str != NULL);
	
	str->str = safe_malloc(STRING_GRANULARITY);
	str->size = STRING_GRANULARITY;
	str->pos = 0;
	
	str->str[str->pos] = 0;
}

void string_clear(string_t *str)
{
	ASSERT(str != NULL);
	ASSERT(str->str != NULL);
	
	str->size = 0;
	str->pos = 0;
	safe_realloc(&str->str, &str->size, str->pos + 1,
	    STRING_GRANULARITY);
	
	str->str[str->pos] = 0;
}

bool string_is_empty(string_t *str)
{
	ASSERT(str != NULL);

	return str->pos == 0;
}

void string_push(string_t *str, char c)
{
	ASSERT(str != NULL);
	ASSERT(str->str != NULL);
	
	safe_realloc(&str->str, &str->size, str->pos + 1,
	    STRING_GRANULARITY);
	
	str->str[str->pos] = c;
	str->pos++;
	str->str[str->pos] = 0;
}

void string_append(string_t *str, const char *val)
{
	ASSERT(str != NULL);
	ASSERT(str->str != NULL);
	ASSERT(val != NULL);
	
	size_t len = strlen(val);
	
	safe_realloc(&str->str, &str->size, str->pos + len,
	    STRING_GRANULARITY);
	
	memcpy(str->str + str->pos, val, len);
	str->pos += len;
	str->str[str->pos] = 0;
}

void string_vprintf(string_t *str, const char *fmt, va_list va)
{
	ASSERT(str != NULL);
	ASSERT(fmt != NULL);
	
	va_list va2;
	va_copy(va2, va);
	int size = vsnprintf(NULL, 0, fmt, va2);
	va_end(va2);
	
	if (size < 0)
		die(ERR_INTERN, "Error formatting string");
	
	safe_realloc(&str->str, &str->size, str->pos + size,
	    STRING_GRANULARITY);
	
	size = vsnprintf(str->str + str->pos, size + 1, fmt, va);
	
	if (size < 0)
		die(ERR_INTERN, "Error formatting string");
	
	str->pos += size;
	str->str[str->pos] = 0;
}

void string_printf(string_t *str, const char *fmt, ...)
{
	ASSERT(str != NULL);
	ASSERT(fmt != NULL);
	
	va_list va;
	
	va_start(va, fmt);
	string_vprintf(str, fmt, va);
	va_end(va);
}

void string_fread(string_t *str, FILE *file)
{
	ASSERT(str != NULL);
	ASSERT(file != NULL);
	
	char buf[STRING_BUFFER];
	
	while (true) {
		if (fgets(buf, STRING_BUFFER, file) == NULL)
			break;
		
		buf[STRING_BUFFER - 1] = 0;
		string_append(str, buf);
	}
}

void string_done(string_t *str)
{
	ASSERT(str != NULL);
	ASSERT(str->str != NULL);
	
	safe_free(str->str);
	str->size = 0;
	str->pos = 0;
}

/** Safe memory allocation
 *
 */
void *safe_malloc(const size_t size)
{
	ASSERT(size > 0);
	
	void *ptr = malloc(size);
	if (ptr == NULL)
		die(ERR_MEM, "Not enough memory");
	
	return ptr;
}

/** Make a copy of a string
 *
 */
char *safe_strdup(const char *str)
{
	ASSERT(str != NULL);
	
	char *duplicate = strdup(str);
	if (duplicate == NULL)
		die(ERR_MEM, "Not enough memory");
	
	return duplicate;
}

/** Test the correctness of the prefix
 *
 */
bool prefix(const char *prefix, const char *string)
{
	ASSERT(prefix != NULL);
	ASSERT(string != NULL);
	
	while (*prefix != 0) {
		if (*prefix != *string)
			return false;
		
		prefix++;
		string++;
	}
	
	return true;
}

/** Safely open a file
 *
 * "Safe" means that an error message is displayed when the file could
 * not be opened.
 *
 * @param path Name of the file to open.
 * @param mode File open mode.
 *
 * @return File structure if successful or NULL otherwise.
 *
 */
FILE *try_fopen(const char *path, const char *mode)
{
	ASSERT(path != NULL);
	ASSERT(mode != NULL);
	
	FILE *file = fopen(path, mode);
	
	if (file != NULL) {
		/* We do not want to open directories */
		if (check_isdir(file)) {
			safe_fclose(file, path);
			file = NULL;
		}
	} else
		io_error(path);
	
	return file;
}

bool check_isdir(FILE *file)
{
	ASSERT(file != NULL);
	
	int fd = fileno(file);
	if (fd == -1) {
		io_error(NULL);
		return false;
	}
	
	struct stat stat;
	if (fstat(fd, &stat) != 0) {
		io_error(NULL);
		return false;
	}
	
	return S_ISDIR(stat.st_mode);
}

/** Safe file close
 *
 * By "safe" we mean printing an error message when the file could not be
 * closed and dying.
 *
 * @param file File to close.
 * @param path Name of the file used for error message.
 *
 */
void safe_fclose(FILE *file, const char *path)
{
	ASSERT(file != NULL);
	ASSERT(path != NULL);
	
	if (fclose(file) != 0)
		io_die(ERR_IO, path);
}

/** Safe fseek
 *
 * This is an implementation of fseek() which displays an error message
 * when the fseek() could not be performed.
 *
 * @param file   File.
 * @param offset Offset to set.
 * @param whence Argument whence for fseek().
 * @param path   Name of the file; used for error message displaying.
 *
 * @return true if successful
 *
 */
bool try_fseek(FILE *file, size_t offset, int whence, const char *path)
{
	ASSERT(file != NULL);
	ASSERT(path != NULL);
	
	if (fseek(file, (long) offset, whence) != 0) {
		io_error(path);
		safe_fclose(file, path);
		return false;
	}
	
	return true;
}

/** Safe ftell
 *
 * This is an implementation of ftell() which displays an error message
 * when the ftell() could not be performed. Also the file position
 * is checked for possible overflows when truncating to the extend
 * of size_t.
 *
 * @param file File.
 * @param path Name of the file; used for error message displaying.
 * @param pos  Returned position.
 *
 * @return true if successful
 *
 */
bool try_ftell(FILE *file, const char *path, size_t *pos)
{
	ASSERT(file != NULL);
	ASSERT(path != NULL);
	ASSERT(pos != NULL);
	
	long lpos = ftell(file);
	*pos = (size_t) lpos;
	
	if ((lpos < 0) || ((long) *pos != lpos)) {
		io_error(path);
		safe_fclose(file, path);
		return false;
	}
	
	return true;
}

void try_munmap(void *ptr, size_t size)
{
	ASSERT(ptr != NULL);
	
	if (munmap(ptr, size) == -1)
		io_error(NULL);
}

/** Convert 32 bit unsigned number to string with k, K, M or no suffix.
 *
 * The conversion applies the first of the following rules
 * which matches the value of converted number:
 *
 * i == 0 -> "0"
 * i can be expressed with the mega suffix -> number of megabytes + M
 * i can be expressed with the kilo suffix -> number of kilobytes + K
 * i is dividable by 1000 -> number of thousands of bytes + k
 * otherwise -> no suffix
 *
 * @param i 32-bit unsigned number to be converted. Expected
 *          to mean count of bytes.
 *
 * @return Allocated string buffer.
 *
 */
char *uint64_human_readable(uint64_t i)
{
	string_t str;
	string_init(&str);
	
	if (i == 0)
		string_push(&str, '0');
	else if ((i & 0x3fffffffU) == 0)
		string_printf(&str, "%" PRIu64 "G", i >> 30);
	else if ((i & 0xfffffU) == 0)
		string_printf(&str, "%" PRIu64 "M", i >> 20);
	else if ((i & 0x3ffU) == 0)
		string_printf(&str, "%" PRIu64 "K", i >> 10);
	else if ((i % 1000) == 0)
		string_printf(&str, "%" PRIu64 "k", i / 1000);
	else
		string_printf(&str, "%" PRIu64, i);
	
	return str.str;
}

/** Test whether the address is double word-aligned
 *
 * @param addr Address to be tested.
 *
 * @return True if the address is 8-byte aligned.
 *
 */
bool ptr36_dword_aligned(ptr36_t addr)
{
	return (addr & 0x07U) == 0;
}

bool ptr36_frame_aligned(ptr36_t addr)
{
	return (addr & FRAME_MASK) == 0;
}

bool virt_range(uint64_t addr)
{
	return true;
}

bool phys_range(uint64_t addr) {
	return (addr <= UINT64_C(0xfffffffff));
}
