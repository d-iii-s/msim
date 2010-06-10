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

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

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
	char *sx;
	
	PRE(str);
	
	sx = strdup(str);
	if (!sx)
		die(ERR_MEM, "Not enough memory");
	
	return sx;
}


/** Test the correctness of the prefix
 *
 */
bool prefix(const char *pref, const char *str)
{
	PRE(pref != NULL, str != NULL);
	
	for (; (*pref) && (*pref == *str); pref++, str++);
	
	return (*pref == '\0');
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
