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

#ifndef CLINE_H_
#define CLINE_H_

#include <stdbool.h>

extern int lineno;
extern char *script_name;
extern bool script_stage;

extern void intr_error(const char *msg, ...);
extern void set_script_stage(const char *sname);
extern void unset_script_stage(void);

#endif /* CLINE_H_ */
