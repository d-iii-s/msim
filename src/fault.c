/*
 * Copyright (c) 2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Fault handlers
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "device/machine.h"
#include "main.h"
#include "fault.h"
#include "utils.h"

/** Script name */
static char *script_name = NULL;

/** Line number */
static size_t lineno = 0;

/** Valid line number */
size_t *lineno_ptr = NULL;

static void verror(const char *fmt, va_list va)
{
	fflush(stdout);
	fprintf(stderr, "<%s> ", PACKAGE);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
	va_list va;
	
	va_start(va, fmt);
	verror(fmt, va);
	va_end(va);
}

void intr_error(const char *msg, ...)
{
	string_t out;
	string_init(&out);
	
	va_list va;
	va_start(va, msg);
	string_printf(&out, msg, va);
	va_end(va);
	
	if (lineno_ptr != NULL) {
		if (script_name)
			error("Error in %s at %zu: %s", script_name, *lineno_ptr, out.str);
		else
			error("Error at %zu: %s", *lineno_ptr, out.str);
	} else
		error("Error: %s", out.str);
	
	string_done(&out);
}

void die(int status, const char *fmt, ...)
{
	va_list va;
	
	va_start(va, fmt);
	verror(fmt, va);
	va_end(va);
	
	input_back();
	exit(status);
}

void io_error(const char *fname)
{
	if (fname)
		error("%s: %s", fname, strerror(errno));
	else
		error("%s", strerror(errno));
}

void io_die(int status, const char *fname)
{
	if (fname)
		die(status, "%s: %s", fname, strerror(errno));
	else
		die(status, "%s", strerror(errno));
}

/** Enter the script stage
 *
 * This enables special error message formating.
 *
 */
void set_script(const char *sname)
{
	lineno_ptr = NULL;
	if (sname)
		script_name = safe_strdup(sname);
}

void set_lineno(size_t no)
{
	lineno = no;
	lineno_ptr = &lineno;
}

/** Leave the script stage
 *
 */
void unset_script(void)
{
	lineno_ptr = NULL;
	safe_free(script_name);
}
