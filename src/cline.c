/*
 * Copyright (c) 2005 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Command line interpretation
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cline.h"
#include "utils.h"
#include "fault.h"


#define BUF_SIZE 1024


/**< Line number */
int lineno = -1;

/**< Script name */
char *script_name = NULL;

/**< Script stage */
bool script_stage = false;


/** Print a formatted error message
 */
void intr_error(const char *msg, ...)
{
	va_list ap;
	char buf[BUF_SIZE];

	va_start(ap, msg);
	vsnprintf(buf, BUF_SIZE, msg, ap);
	va_end(ap);

	if (lineno == -1)
		error("error: %s", buf);
	else {
		if (script_name)
			error("%s(%d): %s", script_name, lineno, buf);
		else
			error("error(%d): %s", lineno, buf);
	}
}


/** Enters the script stage
 *
 * This enables special error message formating.
 *
 */
void set_script_stage(const char *sname)
{
	lineno = -1;
	script_stage = true;
	if (sname)
		script_name = xstrdup(sname);
}


/** Leaves the script stage
 */
void unset_script_stage(void)
{
	script_stage = false;
	XFREE(script_name);
}
