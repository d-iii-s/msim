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
#include <fcntl.h>
#include <inttypes.h>

#include "text.h"
#include "utils.h"
#include "fault.h"
#include "check.h"
#include "main.h"

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
 * @param fd       File descriptor
 * @param flags    Flags of open function
 * @param filename Name of the file to open
 *
 * @return true if successful
 *
 */
bool try_open(int *fd, int flags, const char *filename)
{
	*fd = open(filename, flags);
	if (*fd == -1) {
		io_error(filename);
		return false;
	}
	
	return true;
}

/** Safe file descriptor close
 *
 * By "safe" we mean printing an error message when the file could not be
 * closed.
 *
 * @param fd       File descripton to close
 * @param filename Name of the file used for error message
 *
 * @return true if successful
 *
 */
bool try_close(int fd, const char *filename)
{
	if (close(fd)) {
		io_error(filename);
		return false;
	}
	
	return true;
}

/** Close file descriptor given when I/O error occurs
 *
 * Does not break the program, just only write error message.
 *
 * @param fd       File descriptor to close
 * @param filename Name of the file used for error message
 *
 */
void try_soft_close(int fd, const char *filename)
{
	if (!try_close(fd, filename))
		error(txt_file_close_err);
}

/** Safe lseek
 *
 * This is an implementation of \c lseek which displays an error message
 * when the lseek could not be performed.
 *
 * @param fd       File descriptor
 * @param offset   Offset to set
 * @param whence   Parameter whence of the lseek function call
 * @param filename Name of the file; used for error message displaying
 *
 * @return true if successful
 *
 */
bool try_lseek(int fd, off_t *offset, int whence, const char *filename)
{
	*offset = lseek(fd, *offset, whence);
	if (*offset == (off_t) -1) {
		io_error(filename);
		try_soft_close(fd, filename);
		
		return false;
	}
	
	return true;
}

/** Convert 32 bit unsigned number to string with k, K, M or none suffix.
 *
 * The conversion applies the first of the following rules,
 * which matches the value of converted number:
 * 
 * i == 0 -> "0"
 * i can be expressed with the mega suffix -> number of megabytes + M
 * i can be expressed with the kilo suffix -> number of kilobytes + K
 * i is dividable by 1000 -> number of thousands of bytes + k
 * otherwise -> no suffix
 *
 * @param s Prealocated buffer for the output string.
 * @param i 32 bit unsigned number to be converted. Expected
 *          to mean count of bytes.
 *
 */
void convert_size32_to_readable_string(char *s, uint32_t i)
{
	if (i == 0)
		sprintf(s, "0");
	else if ((i & 0xfffff) == 0)
		sprintf(s, "%" PRId32 "M", i >> 20);
	else if ((i & 0x3ff) == 0)
		sprintf(s, "%" PRId32 "K", i >> 10);
	else if ((i % 1000) == 0)
		sprintf(s, "%" PRId32 "k", i / 1000);
	else
		sprintf(s, "%" PRId32, i);
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
