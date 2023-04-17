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
#include <unistd.h>
#include "../config.h"
#include "input.h"
#include "fault.h"
#include "utils.h"

/** Script name */
static char *script_name = NULL;

/** Line number */
static size_t lineno = 0;

/** Valid line number */
static size_t *lineno_ptr = NULL;

typedef enum {
	COLOR_RED    = 1,
	COLOR_YELLOW = 3,
	COLOR_CYAN   = 6,
	COLOR_WHITE  = 7
} tty_color_t;

typedef enum {
	CMD_RESET = 0,
	CMD_BOLD  = 1,
	CMD_COLOR = 30
} tty_command_t;

static void tty_ctrl(FILE *tty, unsigned int mode)
{
	int fd = fileno(tty);
	
	if ((fd != -1) && (isatty(fd))) {
		fprintf(tty, "\033[%um", mode);
		fflush(tty);
	}
}

static void mverror(unsigned int color, const char *fmt, va_list va)
{
	fflush(stdout);
	tty_ctrl(stderr, CMD_RESET);
	tty_ctrl(stderr, CMD_BOLD);
	tty_ctrl(stderr, CMD_COLOR + color);
	
	fprintf(stderr, "<%s> ", PACKAGE_NAME);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	
	tty_ctrl(stderr, CMD_RESET);
}

static __attribute__((format(printf, 2, 3)))
    void mferror(unsigned int color, const char *fmt, ...)
{
	va_list va;
	
	va_start(va, fmt);
	mverror(color, fmt, va);
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
			mferror(COLOR_YELLOW, "Error in %s on line %zu:\n%s",
			    script_name, *lineno_ptr, out.str);
		else
			mferror(COLOR_YELLOW, "Error on line %zu:\n%s",
			    *lineno_ptr, out.str);
	} else
		mferror(COLOR_YELLOW, "Error: %s", out.str);
	
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
			mferror(COLOR_WHITE, "Internal error in %s on line %zu:\n%s",
			    script_name, *lineno_ptr, out.str);
		else
			mferror(COLOR_WHITE, "Internal error on line %zu:\n%s",
			    *lineno_ptr, out.str);
	} else
		mferror(COLOR_WHITE, "Internal error: %s", out.str);
	
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
			mferror(COLOR_CYAN, "Alert in %s on line %zu:\n%s",
			    script_name, *lineno_ptr, out.str);
		else
			mferror(COLOR_CYAN, "Alert on line %zu:\n%s",
			    *lineno_ptr, out.str);
	} else
		mferror(COLOR_CYAN, "Alert: %s", out.str);
	
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
			mferror(COLOR_RED, "Fault in %s on line %zu:\n%s",
			    script_name, *lineno_ptr, out.str);
		else
			mferror(COLOR_RED, "Fault on line %zu:\n%s",
			    *lineno_ptr, out.str);
	} else
		mferror(COLOR_RED, "Fault: %s", out.str);
	
	string_done(&out);
	
	input_back();
	if (status == ERR_INTERN)
		abort();
	else
		exit(status);
}

void io_error(const char *fname)
{
	if (fname)
		error("%s (%s)", strerror(errno), fname);
	else
		error("%s", strerror(errno));
}

void io_die(int status, const char *fname)
{
	if (fname)
		die(status, "%s (%s)", strerror(errno), fname);
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
