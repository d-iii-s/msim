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
static size_t *lineno_ptr = NULL;

static void mverror(bool nl, const char *fmt, va_list va)
{
	fflush(stdout);
	
	if (nl)
		fprintf(stderr, "\n");
	
	fprintf(stderr, "<%s> ", PACKAGE);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}

static void mferror(bool nl, const char *fmt, ...)
{
	va_list va;
	
	va_start(va, fmt);
	mverror(nl, fmt, va);
	va_end(va);
}

void error(const char *fmt, ...)
{
	string_t out;
	string_init(&out);
	
	va_list va;
	va_start(va, fmt);
	string_vprintf(&out, fmt, va);
	va_end(va);
	
	if (lineno_ptr != NULL) {
		if (script_name)
			mferror(false, "Error in %s at %zu: %s", script_name,
			    *lineno_ptr, out.str);
		else
			mferror(false, "Error at %zu: %s", *lineno_ptr, out.str);
	} else
		mferror(false, "Error: %s", out.str);
	
	string_done(&out);
}

void intr_error(const char *fmt, ...)
{
	string_t out;
	string_init(&out);
	
	va_list va;
	va_start(va, fmt);
	string_vprintf(&out, fmt, va);
	va_end(va);
	
	if (lineno_ptr != NULL) {
		if (script_name)
			mferror(false, "Internal error in %s at %zu: %s", script_name,
			    *lineno_ptr, out.str);
		else
			mferror(false, "Internal error at %zu: %s", *lineno_ptr,
			    out.str);
	} else
		mferror(false, "Internal error: %s", out.str);
	
	string_done(&out);
}

void alert(const char *fmt, ...)
{
	string_t out;
	string_init(&out);
	
	va_list va;
	va_start(va, fmt);
	string_vprintf(&out, fmt, va);
	va_end(va);
	
	if (lineno_ptr != NULL) {
		if (script_name)
			mferror(true, "Alert in %s at %zu: %s", script_name,
			    *lineno_ptr, out.str);
		else
			mferror(true, "Alert at %zu: %s", *lineno_ptr, out.str);
	} else
		mferror(true, "Alert: %s", out.str);
	
	string_done(&out);
}

void die(int status, const char *fmt, ...)
{
	string_t out;
	string_init(&out);
	
	va_list va;
	va_start(va, fmt);
	string_vprintf(&out, fmt, va);
	va_end(va);
	
	if (lineno_ptr != NULL) {
		if (script_name)
			mferror(false, "Fault in %s at %zu: %s", script_name,
			    *lineno_ptr, out.str);
		else
			mferror(false, "Fault at %zu: %s", *lineno_ptr, out.str);
	} else
		mferror(false, "Fault: %s", out.str);
	
	string_done(&out);
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
