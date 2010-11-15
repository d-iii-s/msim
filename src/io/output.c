/*
 * Copyright (c) 2003-2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Output routines
 *
 */

/*
 * We must take a special care with output messages. Some types of messages
 * often exceeds the with of the screen and just simple splitting to more
 * lines may cause unexpected output format.
 *
 * Thus we provide a set of output functions which behaviour depends on
 * the type of information they belong to.
 *
 * Regular text messages are buffered and lines are splitted at word
 * boundaries.
 *
 * This approach works fine in most cases.
 *
 */

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "output.h"
#include "../fault.h"
#include "../check.h"
#include "../parser.h"

/** Output file descriptor */
static FILE *output;

/** Initialize the output management
 *
 */
void output_init(void)
{
	output = stdout;
}

/** Print the message on the screen
 *
 */
void mprintf(const char *fmt, ...)
{
	va_list ap;
	
	PRE(fmt != NULL);
	
	va_start(ap, fmt);
	vfprintf(output, fmt, ap);
	va_end(ap);
}

/** Print the error message
 *
 */
void mprintf_err(const char *fmt, ...)
{
	va_list ap;
	
	PRE(fmt != NULL);
	
	if (lineno_ptr != NULL)
		fprintf(output, "%zu: ", *lineno_ptr);
	
	va_start(ap, fmt);
	vfprintf(output, fmt, ap);
	va_end(ap);
}
