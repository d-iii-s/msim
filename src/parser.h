/*
 * Copyright (c) 2001-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Command line parser
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "list.h"

typedef enum {
	tt_end,
	tt_uint,
	tt_str,
	tt_err,
	tt_err_string_noend,
	tt_err_invalid_num,
	tt_err_invalid_hex_num,
	tt_err_overflow
} token_type_t;

typedef union {
	uint64_t i;
	char *str;
} token_val_t;

typedef struct {
	item_t item;

	token_type_t ttype;
	token_val_t tval;
} token_t;

/** Command parameter types
 *
 * Parameters of a command are represented as a collection strings. Each
 * string describes one parameter, strings are separated by a null character.
 *
 * A parameter description consists of two flags and a name. The first flag
 * determines whether the parameter is required or is optional. The second
 * flag determines the type - integer, string, any type or that a specified
 * sting is required (aka the = mark in the set command).
 *
 * The parameter name may be any string. Also more than one name is allowed,
 * separated by the / character. Then the first variant is used as a short
 * name (acronym) and the second one as full name.
 *
 * Parameters should be separated via the NEXT macro. The END macro should
 * be included at the end of a parameter list.
 */

#define REQ   "r"  /**< Parameter is required */
#define REQC  'r'
#define OPT   "o"  /**< Parameter is optional */
#define OPTC  'o'

#define INT   "i"  /**< Integer */
#define INTC  'i'
#define STR   "s"  /**< String */
#define STRC  's'
#define VAR   "v"  /**< Any */
#define VARC  'v'
#define CON   "c"  /**< Constant required */
#define CONC  'c'

#define NEXT  "\0"  /**< Mark the next parameter */

#define NOCMD   "e"    /**< No command */
#define NOCMDC  'e'
#define CONT    "\0n"  /**< Do not check other params */
#define CONTC   'n'
#define END     "\0e"  /**< Mo more parameter */
#define ENDC    'e'

#define DEFAULT  NULL

/** cmd_find return values */
typedef enum {
	CMP_NO_HIT = 0,        /**< No hit */
	CMP_HIT = 1,           /**< Exact match */
	CMP_PARTIAL_HIT = 2,   /**< Partial match */
	CMP_MULTIPLE_HIT = 3   /**< Multiple match */
} cmd_find_res_t;

extern const char *token_overview[];

struct cmd;

typedef bool (*fcmd_t)(token_t *parm, void *data);
typedef char *(*gen_t)(token_t *parm, const void *data, unsigned int level);
typedef gen_t (*fgen_t)(token_t **parm, const struct cmd *cmd,
    const void **data);

/** Device command list */
typedef struct cmd {
	const char *name;   /* Command name */
	fcmd_t func;        /* Function which implements command */
	fgen_t find_gen;    /* Function for finding the completion function */
	fcmd_t help;        /* Help function */
	const char *desc;   /* Short description */
	const char *descf;  /* Full description */
	const char *pars;   /* Parameters and description */
} cmd_t;

extern token_t *parm_parse(const char *str);
extern void parm_delete(token_t *parm);

extern void parm_check_end(token_t *parm, const char *str);
extern void parm_init(token_t *parm);
extern void parm_set_uint(token_t *parm, uint64_t val);
extern void parm_set_str(token_t *parm, char *str);
extern void parm_insert_uint(token_t *parm, uint64_t val);
extern void parm_insert_str(token_t *parm, char *s);

extern token_t *parm_next(token_t **parm);
extern token_type_t parm_type(token_t *parm);
extern bool parm_last(token_t *parm);
extern uint64_t parm_uint(token_t *parm);
extern char *parm_str(token_t *parm);

extern uint64_t parm_uint_next(token_t **parm);
extern char *parm_str_next(token_t **parm);

extern cmd_find_res_t cmd_find(const char *cmd_name, const cmd_t *cmds,
   const cmd_t **cmd);

extern bool cmd_run_by_spec(const cmd_t *cmd, token_t *parm, void *data);
extern bool cmd_run_by_name(const char *cmd, token_t *parm,
   const cmd_t *cmds, void *data);
extern bool cmd_run_by_parm(token_t *parm, const cmd_t *cmds, void *data);

extern char *generator_cmd(token_t *parm, const void *data,
    unsigned int level);

extern void cmd_print_help(const cmd_t *cmds);
extern void cmd_print_extended_help(const cmd_t *cmds, token_t *parm);

#endif
