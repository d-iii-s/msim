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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "device/device.h"
#include "parser.h"
#include "check.h"
#include "utils.h"
#include "fault.h"

static const cmd_t *last_cmd;

typedef struct {
	token_type_t ttype;
	uint32_t i;
	string_t str;
} int_token_t;

const char *token_overview[] = {
	"end",
	"integer",
	"string",
	"parse error",
	"string too long",
	"unterminated string",
	"invalid number",
	"invalid hexadecimal number",
	"integer overflow"
};

/** Return the multiplication constant
 *
 * The user can specify the multiplication constant.
 * It is a character following the number. The pointer
 * is advanced accordingly.
 *
 */
static unsigned int read_multiply(const char **str)
{
	PRE(str != NULL);
	PRE(*str != NULL);
	
	const char *tmp = *str;
	unsigned int mply;
	
	switch (*tmp) {
	case 'k':
		mply = 1000;
		tmp++;
		break;
	case 'K':
		mply = 1024;
		tmp++;
		break;
	case 'M':
		mply = 1024 * 1024;
		tmp++;
		break;
	case 'G':
		mply = 1024 * 1024 * 1024;
		tmp++;
		break;
	default:
		mply = 1;
	}
	
	*str = tmp;
	return mply;
}

/** Test whether the character is whitespace
 *
 */
static bool whitespace(char c)
{
	return ((c == ' ') || (c == '\t'));
}

/** Test whether the character is a decimal
 *
 * @return True if the character is within 0..9.
 *
 */
static bool decimal(char c)
{
	return ((c >= '0') && (c <= '9'));
}


/** Test whether the character is a hex digit
 *
 *
 */
static bool hexadecimal(char c)
{
	return (((c >= '0') && (c <= '9'))
	    || ((c >= 'a') && (c <= 'f'))
	    || ((c >= 'A') && (c <= 'F')));
}

/** Test whether the character is a digit or alphabetic
 *
 *  @return True if the character is within 0..9, a..z or _.
 *
 */
static bool alphanum(char c)
{
	return (((c >= '0') && (c <= '9'))
	    || ((c >= 'a') && (c <= 'z'))
	    || ((c >= 'A') && (c <= 'Z'))
	    || (c == '_'));
}

/* Move the pointer to the next relevant character
 *
 */
static void skip_whitespace_chars(const char **str)
{
	PRE(str != NULL);
	PRE(*str != NULL);
	
	const char *tmp = *str;
	
	do {
		/* Skip whitespace chars */
		while ((*tmp) && (whitespace(*tmp)))
			tmp++;
		
		/* Skip comments */
		if (*tmp == '#') {
			while ((*tmp) && (*tmp != '\n'))
				tmp++;
			continue;
		}
	} while (false);
	
	*str = tmp;
}

/** Character to integer conversion
 *
 * @return Value of the character.
 *
 */
static unsigned int char2uint(char c)
{
	if ((c >= '0') && (c <= '9'))
		return (c - '0');
	
	if ((c >= 'a') && (c <= 'z'))
		return (c - 'a' + 10);
	
	if ((c >= 'A') && (c <= 'Z'))
		return (c - 'A' + 10);
	
	die(ERR_INTERN, "Character error");
	return 0;
}

/** Convert the text number into the integer
 *
 */
static void read_number(const char **str, int_token_t *token)
{
	PRE(str != NULL, tmp != NULL);
	PRE(*str != NULL);
	
	const char *tmp = *str;
	uint64_t i = 0;
	
	if ((tmp[0] == '0') && (tmp[1] == 'x')) {
		/* Hex number */
		tmp += 2;
		
		while (hexadecimal(*tmp)) {
			i <<= 4;
			i += char2uint(*tmp);
			tmp++;
		}
		
		if (tmp > *str)
			token->ttype = tt_uint;
		else
			token->ttype = tt_err_invalid_hex_num;
	} else {
		/* Decimal number */
		if (decimal(*tmp)) {
			i *= 10;
			i += char2uint(*tmp);
			tmp++;
		}
		
		if (tmp > *str) {
			token->ttype = tt_uint;
			i *= read_multiply(&tmp);
		} else
			token->ttype = tt_err_invalid_num;
	}
	
	/* Overflow test */
	if (i > 0xffffffffU) {
		token->ttype = tt_err_overflow;
		return;
	}
	
	/* Error test */
	if ((*tmp) && (alphanum(*tmp))) {
		token->ttype = tt_err_invalid_num;
		return;
	}
	
	token->i = (uint32_t) i;
	*str = tmp;
}

/** Read a string token
 *
 */
static void read_string(const char **str, int_token_t *token)
{
	PRE(str != NULL, token != NULL);
	PRE(*str != NULL);
	
	const char *tmp = *str;
	string_t out;
	string_init(&out);
	
	if ((*tmp == '"') || (*tmp == '\'')) {
		char quote = *tmp;
		tmp++;
		
		/* Test for quote */
		if (*tmp == quote) {
			tmp++;
		} else {
			while ((*tmp) && (*tmp != quote) && (*tmp != '\n')) {
				string_push(&out, *tmp);
				tmp++;
			}
			
			if (*tmp == '\n') {
				token->ttype = tt_err_string_noend;
				return;
			}
			
			tmp++;
		}
	} else {
		if (!alphanum(*tmp)) {
			tmp++;
		} else {
			while (alphanum(*tmp)) {
				string_push(&out, *tmp);
				tmp++;
			}
		}
	}
	
	*str = tmp;
	token->ttype = tt_str;
	token->str = out;
}

/** Read the next token
 *
 */
static void read_token(const char **str, int_token_t *token)
{
	PRE(str != NULL, token != NULL);
	PRE(*str != NULL);
	
	if ((**str == 0) || (**str == '\n')) {
		token->ttype = tt_end;
		(*str)++;
	} else {
		if (decimal(**str))
			read_number(str, token);
		else
			read_string(str, token);
	}
}

/** Parse next token
 *
 */
static void parse_token(const char **str, int_token_t *token)
{
	PRE(str != NULL, token != NULL);
	PRE(*str != NULL);
	
	skip_whitespace_chars(str);
	read_token(str, token);
}

/** Parse the input line and link all token into the parm list
 *
 */
token_t *parm_parse(const char *str)
{
	PRE(str != NULL);
	
	int_token_t int_token;
	int_token.i = 0;
	int_token.str.str = NULL;
	
	list_t *pars = safe_malloc_t(list_t);
	list_init(pars);
	
	do {
		/* Parse next token */
		parse_token(&str, &int_token);
		
		/* Allocate a new node */
		token_t *token = safe_malloc_t(token_t);
		item_init(&token->item);
		
		/* Copy parameters */
		token->ttype = int_token.ttype;
		switch (int_token.ttype) {
		case tt_uint:
			token->tval.i = int_token.i;
			break;
		case tt_str:
			token->tval.str = int_token.str.str;
			break;
		default:
			break;
		}
		
		list_append(pars, &token->item);
	} while ((int_token.ttype != tt_end) && (int_token.ttype < tt_err));
	
	return ((token_t *) pars->head);
}

/** Delete the parameter list
 *
 * Also all internal data like strings are freed.
 *
 */
void parm_delete(token_t *parm)
{
	PRE(parm != NULL);
	PRE(parm->item.list != NULL);
	
	list_t *list = parm->item.list;
	token_t *token = (token_t *) list->head;
	
	while (token != NULL) {
		if (token->ttype == tt_str)
			safe_free(token->tval.str);
		
		token_t *removed = token;
		token = (token_t *) token->item.next;
		
		list_remove(list, &removed->item);
		safe_free(removed);
	}
}

/** Check for the end of the string
 *
 * If there is a space between the end of the string and the last
 * character, the separator is included into the parameter link.
 *
 */
void parm_check_end(token_t *parm, const char *str)
{
	PRE(parm != NULL);
	PRE(str != NULL);
	
	if (parm_type(parm) == tt_end)
		return;
	
	size_t len = strlen(str);
	
	if ((len > 0) && (whitespace(str[len - 1]))) {
		/* Find the end of the parameter list */
		
		token_t *pre = parm;
		while (parm_type(parm_next(&parm)) != tt_end)
			pre = parm;
		
		parm_insert_str(pre, safe_strdup(""));
	}
}

/** Move the pointer to the next parameter
 *
 */
token_t *parm_next(token_t **parm)
{
	PRE(parm != NULL);
	PRE(*parm != NULL);
	PRE((*parm)->item.next != NULL);
	
	*parm = (token_t *) ((*parm)->item.next);
	return *parm;
}

/** Return the parameter type
 *
 */
token_type_t parm_type(token_t *parm)
{
	PRE(parm != NULL);
	
	return parm->ttype;
}

/** Return true if the parameter is last
 *
 */
bool parm_last(token_t *parm)
{
	PRE(parm != NULL);
	
	if (parm->item.next == NULL)
		return false;
	
	token_t *next = (token_t *) (parm->item.next);
	return (next->ttype == tt_end);
}

/** Return the integer parameter
 *
 */
uint32_t parm_uint(token_t *parm)
{
	PRE(parm != NULL);
	PRE(parm->ttype == tt_uint);
	
	return parm->tval.i;
}

/** Return the string parameter
 *
 */
char *parm_str(token_t *parm)
{
	PRE(parm != NULL);
	PRE(parm->ttype == tt_str);
	
	return parm->tval.str;
}

/** Return the integer parameter and move to the next one
 *
 */
uint32_t parm_next_uint(token_t **parm)
{
	PRE(parm != NULL);
	PRE(*parm != NULL);
	
	uint32_t i = parm_uint(*parm);
	parm_next(parm);
	
	return i;
}

/** Return the integer parameter and moves to the next one
 *
 */
char *parm_next_str(token_t **parm)
{
	PRE(parm != NULL);
	PRE(*parm != NULL);
	
	char *str = parm_str(*parm);
	parm_next(parm);
	
	return str;
}

/** Insert an integer parameter
 *
 * The new parameter is inserted after the specified one.
 *
 */
void parm_insert_uint(token_t *parm, uint32_t val)
{
	PRE(token != NULL);
	
	token_t *ntoken = safe_malloc_t(token_t);
	
	item_init(&ntoken->item);
	ntoken->ttype = tt_uint;
	ntoken->tval.i = val;
	
	list_insert_after(&parm->item, &ntoken->item);
}

/** Insert a string parameter into the parameter list
 *
 * The new parameter is inserted after the specified one.
 *
 */
void parm_insert_str(token_t *parm, char *str)
{
	PRE(token != NULL, str != NULL);
	
	token_t *ntoken = safe_malloc_t(token_t);
	
	item_init(&ntoken->item);
	ntoken->ttype = tt_str;
	ntoken->tval.str = str;
	
	list_insert_after(&parm->item, &ntoken->item);
}

void parm_init(token_t *parm)
{
	PRE(parm != NULL);
	
	item_init(&parm->item);
	parm->ttype = tt_end;
}

/** Change the parameter type to int
 *
 */
void parm_set_uint(token_t *parm, uint32_t val)
{
	PRE(parm != NULL);
	
	if (parm->ttype == tt_str)
		safe_free(parm->tval.str);
	
	parm->ttype = tt_uint;
	parm->tval.i = val;
}

/** Change the parameter type to string
 *
 */
void parm_set_str(token_t *parm, char *str)
{
	PRE(parm != NULL, str != NULL);
	
	if (parm->ttype == tt_str)
		safe_free(parm->tval.str);
	
	parm->ttype = tt_str;
	parm->tval.str = str;
}

/** Find next parameter
 *
 * Return a pointer to the first character following a first terminal.
 * The format is "text\0another text".
 *
 */
static void find_next_parm(const char **str)
{
	PRE(str != NULL);
	PRE(*str != NULL);
	
	while (*(*str)++);
}

/** Compare the command with a name definition.
 *
 * Compare the input command with the command repository.
 *
 * @param str The user input
 * @param cmd Supproted commands.
 *
 * @return CMP_NO_HIT for no match.
 * @return CMP_PARTIAL_HIT for partial match, the specified command
 *         is a prefix of another command.
 * @return CMP_HIT for full match.
 *
 */
static cmd_find_res_t cmd_compare(const char *str, const char *cmd)
{
	PRE(str != NULL, cmd != NULL);
	
	/* Compare strings */
	const char *tmp;
	for (tmp = str; (*cmd) && (*tmp == *cmd); tmp++, cmd++);
	
	/* Check for full match */
	if (*tmp == 0) {
		if (*cmd == 0)
			return CMP_HIT;
		
		return CMP_PARTIAL_HIT;
	}
	
	return CMP_NO_HIT;
}

/** Skip qualifiers from a parameter description
 *
 */
static const char *parm_skipq(const char *str)
{
	PRE(str != NULL);
	
	return (str + 2);
}

/** Check the command name
 *
 * Check the command name against command description structure.
 * Return apropriate value CMD_* and a pointer to the command found.
 *
 * @param cmd_name Name of the command
 * @param cmds     Command structures
 * @param cmd      The command found, NULL means no output
 *
 */
cmd_find_res_t cmd_find(const char *cmd_name, const cmd_t *cmds,
    const cmd_t **cmd)
{
	PRE(cmd_name != NULL, cmds != NULL);
	
	cmd_find_res_t res;
	
	/* Find the command */
	for (res = CMP_NO_HIT; (res != CMP_HIT) && (cmds->name); cmds++) {
		switch (cmd_compare(cmd_name, cmds->name)) {
		case CMP_NO_HIT:
			break;
		case CMP_PARTIAL_HIT:
			if (res == CMP_NO_HIT) {
				res = CMP_PARTIAL_HIT;
				if (cmd)
					*cmd = cmds;
			} else
				res = CMP_MULTIPLE_HIT;
			break;
		case CMP_HIT:
			res = CMP_HIT;
			if (cmd)
				*cmd = cmds;
			break;
		default:
			die(ERR_INTERN, "Command identification error");
		}
	}
	
	return res;
}

/** Find name
 *
 * Search for a long parameter name and return its first character.
 * When no semarator is found the whole string is a long name.
 *
 * @param str The format is "short_name/long_name".
 *
 */
static const char *find_name(const char *str)
{
	PRE(str != NULL);
	
	if ((str[0] == 0) || (str[1] == 0))
		return str;
	
	/* Search for a separator */
	const char *tmp;
	for (tmp = str + 2; (*tmp != 0) && (*tmp != '/'); tmp++);
	
	if (*tmp != 0)
		return (tmp + 1);
	
	return str + 2;
}

static size_t sname_len(const char *str)
{
	PRE(str != NULL);
	
	/* Search for a separator */
	const char *tmp;
	size_t len;
	for (tmp = str, len = 0; (*tmp != 0) && (*tmp != '/'); tmp++, len++);
	
	return len;
}

/** Execute the command passed as the pointer to the command structure
 *
 */
bool cmd_run_by_spec(const cmd_t *cmd, token_t *parm, void *data)
{
	PRE(cmd != NULL, parm != NULL);
	token_t *orig_parm = parm;
	
	/*
	 * Check all parameters.
	 */
	const char *pars = cmd->pars;
	while ((*pars != CONTC) && (*pars != ENDC)) {
		/* Check the first quantifier */
		if ((*pars != OPTC) && (*pars != REQC)) {
			error("Invalid parameter quantifier \"%s\"", pars);
			return false;
		}
		
		if ((*pars == OPTC) && (parm->ttype == tt_end))
			break;
		else {
			if ((*pars == REQC) && (parm->ttype == tt_end)) {
				error("Missing parameter \"%s\"", find_name(pars));
				return false;
			}
		}
		
		/* Check type */
		switch (pars[1]) {
		case INTC:
			if (parm->ttype != tt_uint) {
				error("Invalid argument (integer \"%s\" required)",
				    find_name(pars));
				return false;
			}
			break;
		case STRC:
			if (parm->ttype != tt_str) {
				error("Invalid argument (string \"%s\" required)",
				    find_name(pars));
				return false;
			}
			break;
		case VARC:
			if ((parm->ttype != tt_uint) && (parm->ttype != tt_str)) {
				error("Invalid argument (string or integer \"%s\" required)",
				    find_name(pars));
				return false;
			}
			break;
		case CONC:
			if ((parm->ttype != tt_str)
			    || (strcmp(parm_skipq(pars), parm->tval.str))) {
				error("Invalid argument (string \"%s\" required)",
				    find_name(pars));
				return false;
			}
			break;
		default:
			error("Invalid parameter type \"%s\"", pars);
			return false;
		}
		
		/* Go to next parameter and token */
		find_next_parm(&pars);
		parm_next(&parm);
	}
	
	if ((*pars == ENDC) && (parm->ttype != tt_end)) {
		error("Too many parameters");
		return false;
	}
	
	/*
	 * Check whether the specified function
	 * is already implemented. If so, call it.
	 */
	if (!cmd->func) {
		intr_error("Command not implemented");
		return false;
	}
	
	return cmd->func(orig_parm, data);
}

/** Execute the specified command
 *
 * The command specified by the parameter is searched
 * in the command structure, parameters are verified
 * and if successful, the command si executed.
 *
 */
bool cmd_run_by_name(const char *cmd_name, token_t *parm, const cmd_t *cmds,
    void *data)
{
	PRE(cmd_name != NULL, parm != NULL, cmds != NULL);
	
	const cmd_t *cmd = NULL;
	
	/* Find fine command */
	switch (cmd_find(cmd_name, cmds, &cmd)) {
	case CMP_HIT:
		break;
	case CMP_PARTIAL_HIT:
		intr_error("Partial command \"%s\".", cmd_name);
		return false;
	case CMP_NO_HIT:
		intr_error("Unknown command \"%s\".", cmd_name);
		return false;
	case CMP_MULTIPLE_HIT:
		intr_error("Ambiguous command \"%s\".", cmd_name);
		return false;
	}
	
	return cmd_run_by_spec(cmd, parm, data);
}

/** Apply the specified command
 *
 * The command is specified by the first parameter.
 *
 */
bool cmd_run_by_parm(token_t *parm, const cmd_t *cmds, void *data)
{
	PRE(parm != NULL, cmds != NULL);
	
	/* Check whether the first token is a string */
	if (parm_type(parm) != tt_str) {
		intr_error("Command name string expected");
		return false;
	}
	
	const char *cmd_name = parm_next_str(&parm);
	return cmd_run_by_name(cmd_name, parm, cmds, data);
}

/** Command tab completion generator
 *
 * @param level The call index number. Used only for the first
 *              call when equal to 0.
 *
 */
char *generator_cmd(token_t *parm, const void *data, unsigned int level)
{
	PRE(parm != NULL, data != NULL);
	PRE((parm_type(parm) == tt_str) || (parm_type(parm) == tt_end));
	
	if (level == 0)
		last_cmd = (cmd_t *) data;
	
	/* Find command */
	const char *cmd_prefix = (parm_type(parm) == tt_str) ?
	    parm_str(parm) : "";
	
	/* Device command list is ended by LAST_CMD */
	while (last_cmd->name) {
		if (prefix(cmd_prefix, last_cmd->name))
			break;
		
		last_cmd++;
	}
	
	if (last_cmd->name == NULL)
		return NULL;
	
	char *name = safe_strdup(last_cmd->name);
	last_cmd++;
	
	return name;
}

static char *cmd_get_syntax(const cmd_t *cmd)
{
	string_t msg;
	string_init(&msg);
	
	string_append(&msg, cmd->name);
	
	unsigned int opt = 0;
	const char *pars = cmd->pars;
	
	while ((*pars != ENDC) && (*pars != CONTC)) {
		/* Append a space */
		string_push(&msg, ' ');
		
		/* Check optional */
		if (*pars == OPTC) {
			string_push(&msg, '[');
			opt++;
		}
		
		/* Short parameter name */
		if (pars[1] != CONC) {
			char *name = safe_strdup(parm_skipq(pars));
			name[sname_len(name)] = 0;
			
			string_printf(&msg, "<%s>", name);
			safe_free(name);
		} else
			string_append(&msg, parm_skipq(pars));
		
		find_next_parm(&pars);
	}
	
	/* Close brackets */
	for (; opt; opt--)
		string_push(&msg, ']');
	
	return msg.str;
}

/** Print a help text
 *
 * Print a help on the screen generated from the command structure.
 *
 * @param cmds commands
 *
 */
void cmd_print_help(const cmd_t *cmds)
{
	PRE(cmds != NULL);
	
	/* Ignore the hardwired "init" command */
	cmds++;
	
	printf("[Command and arguments       ] [Description\n");
	
	while (cmds->name) {
		char *syntax = cmd_get_syntax(cmds);
		printf("%-30s %s\n", syntax, cmds->desc);
		safe_free(syntax);
		cmds++;
	}
}

/** Print help text about a command.
 *
 * @param parm Parameters (at most one string).
 * @param cmds The array of supported commands.
 *
 */
void cmd_print_extended_help(const cmd_t *cmds, token_t *parm)
{
	PRE(parm != NULL, cmds != NULL);
	
	if (parm_type(parm) == tt_end) {
		cmd_print_help(cmds);
		return;
	}
	
	const char *cmd_name = parm_str(parm);
	const cmd_t *cmd = NULL;
	
	/* Find command */
	switch (cmd_find(cmd_name, cmds, &cmd)) {
	case CMP_HIT:
		break;
	case CMP_PARTIAL_HIT:
		intr_error("Partial command \"%s\".", cmd_name);
		return;
	case CMP_NO_HIT:
		intr_error("Unknown command \"%s\".", cmd_name);
		return;
	case CMP_MULTIPLE_HIT:
		intr_error("Ambiguous command \"%s\".", cmd_name);
		return;
	}
	
	printf("%s\n", cmd->descf);
	
	char *syntax = cmd_get_syntax(cmd);
	printf("Syntax: %s\n", syntax);
	safe_free(syntax);
	
	const char *pars = cmd->pars;
	while ((*pars != ENDC) && (*pars != CONTC)) {
		if (pars[1] != CONC) {
			char *name = safe_strdup(parm_skipq(pars));
			name[sname_len(name)] = 0;
			
			printf("\t<%s> %s\n", name, find_name(pars));
			safe_free(name);
		}
		
		find_next_parm(&pars);
	}
}
