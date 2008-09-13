/*
 * Small useful routines
 *
 * Copyright (c) 2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "fault.h"
#include "check.h"
#include "mcons.h"


/** Safe memory allocation.
 */
void *
xmalloc( size_t s)

{
	void *v = malloc( s);
	if (!v)
		die( ERR_MEM, "Not enough memory");
	return v;
}


/** Makes a copy of a string.
 */
char *
xstrdup( const char *s)

{
	char *sx;
	
	PRE( s);
	
	sx = strdup( s);
	if (!sx)
		die( ERR_MEM, "Not enough memory");
	return sx;
}


/** Tests the correctness of the prefix.
 */
bool
prefix( const char *pref, const char *str)

{
	PRE( pref != NULL, str != NULL);

	for (; *pref && *pref == *str; pref++, str++) ;

	return *pref == '\0';
}
