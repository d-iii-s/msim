/*
 * Command line interpretation
 *
 * Copyright (c) 2005 Viliam Holub
 */

#ifndef _CLINE_H_
#define _CLINE_H_

#include <stdbool.h>

extern int lineno;
extern char *script_name;
extern bool script_stage;

void intr_error( const char *msg, ...);
void set_script_stage( const char *sname);
void unset_script_stage( void);

#endif /* _CLINE_H_ */
