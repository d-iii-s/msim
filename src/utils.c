/*
 * Small useful routines
 *
 * Copyright (c) 2004 Viliam Holub
 */

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "fault.h"
#include "check.h"

void *xmalloc( size_t s)

{
	void *v = malloc( s);
	if (!v)
		die( FAULT_NOMEM, "Not enough memory");
	return v;
}


char *xstrdup( const char *s)

{
	char *sx = strdup( s);
	if (!sx)
		die( FAULT_NOMEM, "Not enough memory");
	return sx;
}


/** Tests the correctness of the prefix.
 */
bool
prefix( const char *pref, const char *str)

{
	REQUIRED( pref != NULL, str != NULL);

	for (; *pref && *pref == *str; pref++, str++) ;

	return *pref == '\0';
}
