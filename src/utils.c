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
