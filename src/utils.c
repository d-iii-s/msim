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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "text.h"
#include "utils.h"
#include "fault.h"
#include "check.h"
#include "main.h"

#define MAX_STRING  16

/** Safe memory allocation
 *
 */
void *safe_malloc(const size_t size)
{
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
	PRE(str != NULL);
	
	char *duplicate = strdup(str);
	if (!duplicate)
		die(ERR_MEM, "Not enough memory");
	
	return duplicate;
}

/** Test the correctness of the prefix
 *
 */
bool prefix(const char *prefix, const char *string)
{
	PRE(prefix != NULL, string != NULL);
	
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
	FILE *file = fopen(path, mode);
	
	if (file == NULL)
		io_error(path);
	
	return file;
}

/** Safe file close
 *
 * By "safe" we mean printing an error message when the file could not be
 * closed.
 *
 * @param file File to close.
 * @param path Name of the file used for error message.
 *
 * @return true if successful
 *
 */
bool try_fclose(FILE *file, const char *path)
{
	if (fclose(file) != 0) {
		io_error(path);
		return false;
	}
	
	return true;
}

/** Close file when I/O error occurs
 *
 * Does not stop the program, only writes an error message.
 *
 * @param file File to close.
 * @param path Name of the file used for error message.
 *
 */
void try_soft_fclose(FILE *file, const char *path)
{
	if (!try_fclose(file, path))
		error(txt_file_close_err);
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
	if (fseek(file, (long) offset, whence) != 0) {
		io_error(path);
		try_soft_fclose(file, path);
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
	PRE(pos != NULL);
	
	long lpos = ftell(file);
	*pos = (size_t) lpos;
	
	if ((lpos < 0) || ((long) *pos != lpos)) {
		io_error(path);
		try_soft_fclose(file, path);
		return false;
	}
	
	return true;
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
char *uint32_human_readable(uint32_t i)
{
	char *str = safe_malloc(MAX_STRING);
	
	if (i == 0)
		snprintf(str, MAX_STRING, "0");
	else if ((i & 0xfffff) == 0)
		snprintf(str, MAX_STRING, "%" PRIu32 "M", i >> 20);
	else if ((i & 0x3ff) == 0)
		snprintf(str, MAX_STRING, "%" PRIu32 "K", i >> 10);
	else if ((i % 1000) == 0)
		snprintf(str, MAX_STRING, "%" PRIu32 "k", i / 1000);
	else
		snprintf(str, MAX_STRING, "%" PRIu32, i);
	
	str[MAX_STRING - 1] = 0;
	return str;
}

/** Test whether the address is word-aligned
 *
 * @param addr Address to be tested.
 *
 * @return True if the address is 4B aligned.
 *
 */
bool addr_word_aligned(ptr_t addr)
{
	return (addr & 0x3) == 0;
}
