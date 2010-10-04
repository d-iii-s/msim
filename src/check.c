/*
 * Copyright (c) 2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Pre/post conditions
 *
 * See check.h for additional information.
 *
 * The auxiliary function checks conditions and displays an error message.
 * Regarding to the specified parameter the program may be halted.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "check.h"

#ifdef RQ_DEBUG

#define RQ_COND_NO_SIZE  128

#define COL_FILENAME   0
#define COL_LINENO     1
#define COL_FUNCNAME   2
#define COL_TEXT       3
#define COL_CONDITION  4
#define COL_NORMAL     5
#define COL_CONDNO     6
#define COL_END        7

static char *colors[COL_END][2] = {
	{"", "\x1b\x5b\x31\x3b\x33\x36\x6d"},
	{"", "\x1b\x5b\x33\x34\x6d"},
	{"", "\x1b\x5b\x31\x3b\x33\x31\x6d"},
	{"", "\x1b\x5b\x30\x6d"},
	{"", "\x1b\x5b\x31\x3b\x33\x31\x6d"},
	{"", "\x1b\x5b\x30\x6d"},
	{"", "\x1b\x5b\x31\x3b\x33\x31\x6d"}
};

void rq_test(const char *pre, const char *filename, unsigned int lineno,
    const char *func, const char *term, ...)
{
	unsigned int i = 0;
	unsigned int cnt = 0;
	int cond;
	va_list va;
	
	/* Check conditions */
	va_start(va, term);
	do {
		cond = va_arg(va, int);
		i++;
		cnt++;
	} while ((cond) && (cond != RQ_PARM_BRK));
	va_end(va);
	
	/* If there is one which does not hold */
	if (cond != RQ_PARM_BRK) {
		fflush(stdout);
		
		bool color = false;
		
#ifdef RQ_COLOR
		if (isatty(fileno(stderr)))
			color = true;
#endif
		
		char cond_no[RQ_COND_NO_SIZE];
		cond_no[0] = 0;
		
		if (cnt != 1)
			snprintf(cond_no, RQ_COND_NO_SIZE, "the %s%d.%s ",
			    colors[COL_CONDNO][color], i, colors[COL_TEXT][color]);
		
		fprintf(stderr, "\n%s%s%s(%s%d%s): in %s%s()%s %s%scondition %s"
		    "%s\"%s\"%s does not hold%s\n",
		    colors[COL_FILENAME][color], filename, colors[COL_TEXT][color],
		    colors[COL_LINENO][color], lineno, colors[COL_TEXT][color],
		    colors[COL_FUNCNAME][color], func, colors[COL_TEXT][color],
		    cond_no, pre, cond_no[0] ? "of " : "",
		    colors[COL_CONDITION][color], term, colors[COL_TEXT][color],
		    colors[COL_NORMAL][color]);
		
#ifdef RQ_FATAL
		exit(RQ_FATAL);
#endif
	}
}

#endif /* RQ_DEBUG */
