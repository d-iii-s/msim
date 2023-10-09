/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Environment variables
 *
 */

#ifndef ENV_H_
#define ENV_H_

#include <stdbool.h>
#include "main.h"
#include "parser.h"

/** System variables */
extern bool iaddr;
extern bool iopc;
extern bool icmt;
extern bool iregch;
extern unsigned int r4k_ireg;

/**< Variable types */
typedef enum {
    vt_uint,
    vt_str,
    vt_bool
} var_type_t;

/** System command implementation */
extern bool env_cmd_set(token_t *parm);
extern bool env_cmd_unset(token_t *parm);

/** Routines */
extern unsigned int env_cnt_partial_varname(const char *name);
extern bool env_check_varname(const char *name, var_type_t *type);
extern bool env_check_type(const char *name, var_type_t type);

/** TAB completion */
extern char *generator_env_name(token_t *parm, const void *data,
    unsigned int level);
extern char *generator_env_booltype(token_t *parm, const void *data,
    unsigned int level);
extern char *generator_bool_envname(token_t *parm, const void *data,
    unsigned int level);
extern char *generator_equal_char(token_t *parm, const void *data,
    unsigned int level);

#endif
