/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Reading and executing commands
 *
 */

#ifndef CMD_H_
#define CMD_H_

#include "parser.h"

extern bool interpret(const char *str);
extern void script(void);
extern gen_t find_completion_generator(token_t **parm, const void **data);

#endif
