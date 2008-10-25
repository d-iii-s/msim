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

#include "parser.h"

#include "mtypes.h"
#include "io/output.h"
#include "check.h"
#include "utils.h"
#include "device/device.h"
#include "cline.h"

#define SNAME_SIZE 32


struct g_token_s {
	enum token_enum ttype;
	uint32_t i;
	char s[ PARSER_MAX_STR];
};
typedef struct g_token_s g_token_s;


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


parm_link_s pars_end = {
	{
		tt_end,
		{
			0
		}
	},
	NULL
};


/** Return the multiplication constant
 *
 * The user can specify the multiplication constant. It is a character
 * following the number.
 * 
 * SE Moves the pointer to the next character if matches.
 *
 */
static int multiply_char(char c)
{
	switch (c) {
	case 'k': return 1000;
	case 'K': return 1024;
	case 'M': return 1024 * 1024;
	}
	
	return 1;
}


/** Test whether the character is white
 * 
 * White chars are spaces, tabs and comments.
 *
 */
static bool white(char c)
{
	return ((c == ' ') || (c == '\t'));
}


/** Test whether the character is a decimal
 *
 * @return true if the character is within 0..9
 *
 */
static bool decimal(char c)
{
	return ((c >= '0') && (c <= '9'));
}


/** A "hex number" test
 *
 * @return true if the character is within 0..9, a..f
 *
 */
bool hexadecimal( char c)
{
	return (((c >= '0') && (c <= '9'))
		|| ((c >= 'a') && (c <= 'f'))
		|| ((c >= 'A') && (c <= 'F')));
}


/** A "character or number" test
 *
 *  @return true if the character is within 0..9, a..z or _
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
static void skip_white_chars(const char **s)
{
	const char *c = *s;
	
	PRE(s != NULL);
	PRE(*s != NULL);

	do {
		/* Skip white chars */
		while ((*c) && (white(*c)))
			c++;

		/* Skip comments */
		if (*c == '#') {
			while ((*c) && (*c != '\n'))
				c++;
			continue;
		}
	} while (false);

	*s = c;
}

/** A character to integer conversion
 *
 * @return A value of the character, -1 if error
 *
 */
int hex2int(char c)
{
	if ((c >= '0') && (c <= '9'))
		return (c - '0');
	
	if ((c >= 'a') && (c <= 'z'))
		return (c - 'a' + 10);
	
	if ((c >= 'A') && (c <= 'Z'))
		return (c - 'A' + 10);
	
	return -1;
}


/** Convert the text number into the integer
 *
 */
static void read_number(const char **s, g_token_s *t)
{
	const char *c = *s;
	uint32_t i;
	volatile uint32_t oi, oi2;

	PRE(s != NULL, t != NULL);
	PRE(*s != NULL);

	oi = 0;
	i = 0;
	
	if ((*c == '0') && (*(c + 1) == 'x')) {
		/* Hex number */
		c += 2;
		if (!hexadecimal(*c)) {
			t->ttype = tt_err_invalid_hex_num;
			return;
		}

		for (; (oi <= i) && (hexadecimal(*c)); c++) {
			oi = i;
			i = i * 16 + hex2int(*c);
		}
	} else {
		/* Dec number */
		for (; oi <= i && (decimal(*c)); c++) {
			oi = i;
			i = i * 10 + (*c - '0');
		}
	}
	
	/* Multiply */
	oi2 = i;
	i *= multiply_char(*c);
	c += oi2 != i;
	
	/* Overflow test */
	if ((oi > i) || (oi2 > i)) {
		t->ttype = tt_err_overflow;
		return;
	}

	t->i = i;
	*s = c;

	/* Error test */
	if ((*c) && (alphanum(*c)))
		t->ttype = tt_err_invalid_num;
	else
		t->ttype = tt_int;
}


/** Read a string token
 *
 * The size limit of the string token is specified by the PARSER_MAX_STR
 * constant.
 *
 * @param s Pointer to the input token
 * @param t A token structure to store the string
 *
 */
static void read_string(const char **s, g_token_s *t)
{
	int i2 = 0;
	const char *c = *s;
	char *c2 = t->s;
	char cx;

	PRE(s != NULL, t != NULL);
	PRE(*s != NULL);

	if ((*c == '"') || (*c == '\'')) {
		cx = *c;
		
		/* Test for a "" */
		if (*(++c) == cx)
			c++;
		else {
			/* Reading hard string */
			while ((*c) && (*c != cx) && (*c != '\n')
				&& (i2 < PARSER_MAX_STR - 1)) {
				*c2 = *c;
				c++;
				c2++;
				i2++; 
			}

			/* EOL test */
			if (*c == '\n') {
				t->ttype = tt_err_string_noend;
				return;
			}

			/* Length test */
			if (i2 == PARSER_MAX_STR - 1) {
				t->ttype = tt_err_string_too_long;
				return;
			}

			c++;
		}
	} else /* Special character test */
		if (!alphanum(*c)) {
			*c2 = *c;
			c++;
			c2++;
		} else {
			/* Read string to the next non-string character */
			while ((alphanum(*c)) && (i2 < PARSER_MAX_STR - 1)) {
				*c2 = *c;
				c++;
				c2++;
				i2++;
			}
			
			/* Length test */
		if (i2 == PARSER_MAX_STR - 1) {
			t->ttype = tt_err_string_too_long;
			return;
		}
	}
	
	*s = c;
	*c2 = 0;
	t->ttype = tt_str;
}


/** Read the next token
 *
 * @param s Pointer to the current position
 * @param t A structure to store the next token
 *
 */
static void read_token(const char **s, g_token_s *t)
{
	const char *c = *s;

	PRE(s != NULL, t != NULL);
	PRE(*s != NULL);
	
	/* End? */
	if ((*c == 0) || (*c == '\n')) {
		t->ttype = tt_end;
		(*s)++;
	} else
		/* Number? */
		if (decimal(*c))
			read_number(s, t);
		else
			/* String */
			read_string(s, t);
}


/** Upload next token
 *
 * SE The position pointer is moved to the character next to the token.
 *
 * @param s The pointer to the current position
 * @param t Output token
 *
 */
static void parse_g_next(const char **s, g_token_s *t)
{
	PRE(s != NULL, t != NULL);
	PRE(*s != NULL);

	skip_white_chars(s);
	read_token(s, t);
}


/** Make a string duplicate up to the speficied size
 *
 * It is similar to the GNU extension variant.
 *
 */
static char *strndup(const char *s, size_t max)
{
	size_t len;
	char *r;

	PRE(s != NULL);

	for (len = 0; (s[len]) && (len < max); len++);
	
	r = safe_malloc(len + 1);
	memcpy(r, s, len);
	r[len] = '\0';

	return r;
}


/** Delete the parameter list
 *
 * Also all internal data like strings are freed.
 *
 * @param pl parm structure, may be NULL (no operation is performed)
 *
 */
void parm_delete(parm_link_s *pl)
{
	parm_link_s *p;

	for (p = pl; p; p = pl) {
		pl = p->next;

		if (p->token.ttype == tt_str)
			free(p->token.tval.s);
		free(p);
	}
}


/** Parse the input line and link all token into the parm list
 *
 * @return A pointer to the first element, otherwise NULL as a not-enough-memory
 *         error.
 *
 */
parm_link_s *parm_parse(const char *s)
{
	token_s *t;
	parm_link_s *pl, **p = &pl;
	g_token_s gt;

	PRE(s != NULL);
	
	do {
		/* Parse next token */
		parse_g_next(&s, &gt);
		
		/* Allocate a new node */
		*p = (parm_link_s *) safe_malloc_t(parm_link_s);
		
		/* Copy parameters */
		t = &(*p)->token;
		t->ttype = gt.ttype;
		switch (t->ttype) {
		case tt_int:
			t->tval.i = gt.i;
			break;
		case tt_str:
			t->tval.s = safe_strdup(gt.s);
			if (!t->tval.s) {
				/* Memory allocation error */
				t->ttype = tt_err;
				parm_delete(pl);
				return NULL;
			}
			break;
		default:
			break;
		}
		(*p)->next = NULL;
		
		p = &(*p)->next;
	} while ((gt.ttype != tt_end) && (gt.ttype < tt_err));
	
	return pl;
}


/** Check for the end of the string
 *
 * If there is a space between the end of the input string and the last
 * character, the separator is included into the parameter link.
 *
 */
void parm_check_end(parm_link_s *pl, const char *input)
{
	parm_link_s *plo;
	size_t sl;

	PRE(pl != NULL);
	
	if (parm_type(pl) == tt_end)
		return;
	
	sl = strlen(input);

	if ((sl) && (white(input[sl-1]))) {
		/* Find the end of the parameter list */
		plo = pl;
		while (parm_type(parm_next(&pl)) != tt_end)
			plo = pl;

		parm_insert_str(plo, safe_strdup(""));
	}
}


/** Find a long name
 * 
 * Search for a long parameter name and returns its first character.
 * When no semarator is found the whole string is a long name.
 *
 * @param s Input string. The format is "short_name/long_name".
 *
 */
static const char *find_lname(const char *s)
{
	const char *s2;

	PRE(s != NULL);
	
	/* Search for a separator */
	s += 2;
	for (s2 = s; (*s2) && (*s2 != '/'); s2++);
	
	return ((*s2) ? s2 + 1 : s);
}


/** Find a short name
 * 
 * Search for a short parameter name and returns its length.
 * When no semarator is found the whole string is a short name.
 *
 * @param s Input string. The format is "short_name/long_name".
 * @return Size of the name.
 *
 */
static int find_sname_len(const char *s)
{
	const char *s2;

	PRE(s != NULL);
	
	/* search for a separator */
	s += 2;
	for (s2 = s; (*s2) && (*s2 != '/'); s2++);
	
	return (s2 - s);
}


/** Find next parameter
 * 
 * Return a pointer to the first character following a first terminal.
 *
 * @param s The input string. The format is "text\0another text".
 *
 */
static const char *find_next_parm(const char *s)
{
	PRE(s != NULL);

	while (*s++);
	
	return s;
}


/** Allocate a new string containing the long name
 *
 * @param lname A string of format "long_name/short_name" or just only
 *              the long string.
 *
 */
static char *dup_lname(const char *lname)
{
	int len;

	PRE(lname);
	PRE(*lname != '\0', *lname != '/');

	for (len = 0; (lname[len]) && (lname[len] != '/'); len++);

	return strndup(lname, len);
}


/** Compare the command with a name definition.
 *
 * Compare the input command with the command repository.
 *
 * @param s   The user input
 * @param cmd Supproted commands. Command names/variants are separated via
 *            the '/' character.
 *
 * @return CMP_NH - no hit
 *         CMP_PHIT - partial hit, the specified command is a prefix of another command
 *         CMP_HIT - full hit
 *
 */
static int cmdcmp(const char *s, const char *cmd)
{
	int phit = 0;
	const char *s2;

	PRE(s != NULL, cmd != NULL);

	do {
		if (*cmd == '/')
			cmd++;
		
		/* Compare strings */
		for (s2 = s; (*cmd) && (*cmd != '/') && (*cmd == *s2); cmd++, s2++);
		
		/* Check full hit */
		if (!*s2) {
			if ((!*cmd) || (*cmd == '/'))
				return CMP_HIT;
			phit++;
		}
		
		/* Go to the next command variation */
		while ((*cmd) && (*cmd != '/'))
			cmd++;
	} while (*cmd);

	return (phit ? CMP_PHIT : CMP_NH);
}


/** Compare the parial name with the long command name
 *
 */
static bool cmd_lprefix(const char *par_name, const cmd_s *cmd)
{
	const char *s;
	const char *s2;

	PRE(par_name != NULL, cmd != NULL);
	PRE(*par_name != '/');

	/* Compare strings */
	s = par_name;
	s2 = cmd->name;
	while ((*s) && (*s2 != '/') && (*s == *s2)) {
		s++;
		s2++;
	}

	return ((*s2 == '/') || (*s == '\0'));
}


/** Move the pointer to the next parameter
 *
 */
parm_link_s *parm_next(parm_link_s **pl)
{
	PRE(pl != NULL);
	PRE(*pl != NULL);

	return (*pl = (*pl)->next);
}


/** Return the parameter type
 *
 */
int parm_type(parm_link_s *parm)
{
	PRE(parm != NULL);
	
	return parm->token.ttype;
}


/** Return the integer parameter
 *
 */
uint32_t parm_int(parm_link_s *parm)
{
	PRE(parm != NULL);
	
	return parm->token.tval.i;
}


/** Return the string parameter
 *
 */
char *parm_str(parm_link_s *parm)
{
	PRE(parm != NULL);
	
	return parm->token.tval.s;
}


/** Return the integer parameter and move to the next one
 *
 */
uint32_t parm_next_int(parm_link_s **parm)
{
	uint32_t u;

	PRE(parm != NULL);
	PRE(*parm != NULL);
	
	u = (*parm)->token.tval.i;
	*parm = (*parm)->next;

	return u;
}


/** Return the integer parameter and moves to the next one
 *
 */
char *parm_next_str(parm_link_s **parm)
{
	char *s;

	PRE(parm != NULL);
	PRE(*parm != NULL);

	s = (*parm)->token.tval.s;
	*parm = (*parm)->next;

	return s;
}


/** Insert an integer parameter
 *
 * A new parameter is inserted NEXT TO the specified one.
 *
 */
bool parm_insert_int(parm_link_s *pl, uint32_t val)
{
	parm_link_s *p;

	PRE(pl != NULL);

	p = safe_malloc_t(token_s);

	p->token.ttype = tt_int;
	p->token.tval.i = val;
	p->next = pl->next;
	pl->next = p;

	return true;
}


/** Insert a string parameter into the parameter list
 *
 * A new parameter is inserted NEXT TO the specified one.
 *
 */
bool parm_insert_str(parm_link_s *pl, char *s)
{
	parm_link_s *p;
	
	PRE(pl != NULL, s != NULL);

	p = safe_malloc_t(token_s);

	p->token.ttype = tt_str;
	p->token.tval.s = s;
	p->next = pl->next;
	pl->next = p;
	
	return true;
}


/** Change the parameter type to int
 *
 */
void parm_change_int(parm_link_s *parm, uint32_t val)
{
	PRE(parm != NULL);

	if (parm->token.ttype == tt_str)
		free(parm->token.tval.s);
	parm->token.ttype = tt_int;
	parm->token.tval.i = val;
}


/** Set a token to a string
 *
 * Set the item of a parameter list to specify a string token.
 *
 */
void parm_set_str(parm_link_s *pl, const char *s)
{
	PRE(pl != NULL, s != NULL);

	pl->token.ttype = tt_str;
	strcpy(pl->token.tval.s, s);
}


/** Skip qualifiers from a parameter description
 *
 */
static const char *parm_skipq(const char *s)
{
	PRE(s != NULL);

	return (s + 2);
}


/** Check the command name
 *
 * Check the command name against command description structure.
 * Return apropriate value CMD_* and a pointer to the found command.
 *
 * @param cmd_name Name of the command
 * @param cmds     Command structures
 * @param cmd      The found command, NULL means no output
 *
 */
int cmd_find(const char *cmd_name, const cmd_s *cmds, const cmd_s **cmd)
{
	int phit;

	PRE(cmd_name != NULL, cmds != NULL);

	/* Find fine command */
	for (phit = 0; (phit != -1) && (cmds->name); cmds++)
		switch (cmdcmp(cmd_name, cmds->name)) {
		case CMP_NH:
			break;
		case CMP_PHIT:
			if ((phit++ == 0) && (cmd))
				*cmd = cmds;
			break;
		case CMP_HIT:
			phit = -1;
			if (cmd)
				*cmd = cmds;
			break;
		}
	
	/* Count output */
	switch (phit) {
	case -1:
		return CMP_HIT;
	case 0:
		return CMP_NH;
	case 1:
		return CMP_HIT;
	}

	return CMP_MHIT;
}


/** Execute the command passed as the pointer to the command structure
 *
 */
bool cmd_run_by_spec(const cmd_s *cmd, parm_link_s *parm, void *data)
{
	const char *s;
	parm_link_s *p = parm;

	PRE(cmd != NULL, parm != NULL);

	/*
	 * Now we have to go over all parameters and check them.
	 */
	s = cmd->pars;
	while ((*s != CONTC) && (*s != ENDC)) {
		/* Check the first quantifier */
		if ((*s != OPTC) && (*s != REQC)) {
			mprintf_err("Internal error (invalid parameter quantifier \"%s\")\n", s);
			return false;
		}
		
		if ((*s == OPTC) && (p->token.ttype == tt_end))
			break;
		else
			if ((*s == REQC) && (p->token.ttype == tt_end)) {
				mprintf_err("Missing parameter <%s>\n",	find_lname(s));
				return false;
			}

		/* Check type */
		switch (*(s + 1)) {
		case INTC:
			if (p->token.ttype != tt_int) {
				mprintf_err("Invalid argument (integer <%s> required)\n", find_lname(s));
				return false;
			}
			break;
		case STRC:
			if (p->token.ttype != tt_str) {
				mprintf_err("Invalid argument (string <%s> required)\n", find_lname(s));
				return false;
			}
			break;
		case VARC:
			if ((p->token.ttype != tt_int) && (p->token.ttype != tt_str)) {
				mprintf_err("Invalid argument (string or integer <%s> required)\n", find_lname(s));
				return false;
			}
			break;
		case CONC:
			if ((p->token.ttype != tt_str) || (strcmp(parm_skipq(s), p->token.tval.s))) {
				mprintf_err("Invalid argument (string <%s> required)\n", find_lname(s));
				return false;
			}
			break;
		default:
			mprintf_err("Internal error (invalid parameter type)\n");
			return false;
		}
		
		s = find_next_parm(s);  /* Next parameter */
		parm_next(&p);          /* Next token */
	}

	if ((*s == ENDC) && (p->token.ttype != tt_end)) {
		mprintf_err("Too many parameters\n");
		return false;
	}
	
	/*
	 * Check whether the specified function
	 * is already implemented. If so, call it.
	 */
	if (!cmd->func) {
		intr_error("Command not implemented.\n");
		return false;
	}
	
	return cmd->func(parm, data); 
}


/** Execute the specified command
 * 
 * The command specified by the parameter is searched in the command structure,
 * parameters are verified and if successful, the command si executed.
 *
 */
bool cmd_run_by_name(const char *cmd_name, parm_link_s *parm, const cmd_s *cmds, void *data)
{
	const cmd_s *cmd;

	PRE(cmd_name != NULL, parm != NULL, cmds != NULL);

	/* Find fine command */
	switch (cmd_find(cmd_name, cmds, &cmd)) {
	case CMP_NH:
		intr_error("Unknown command: %s", cmd_name);
		return false;
	case CMP_MHIT:
		intr_error("Ambiguous command: %s", cmd_name);
		return false;
	}
	
	return cmd_run_by_spec(cmd, parm, data);
}


/** Apply the specified command
 * 
 * The command is specified by the first parameter.
 *
 */
bool cmd_run_by_parm(parm_link_s *pl, const cmd_s *cmds, void *data)
{
	PRE(pl != NULL, cmds != NULL);

	/* Check whether the first token is a string */
	if (pl->token.ttype != tt_str) {
		intr_error("Command name expected.");
		return false;
	}
	
	return cmd_run_by_name(pl->token.tval.s, pl->next, cmds, data);
}


/** Command tab completion generator
 * 
 * @param level The call index number. Used only for the first
 *              call when equal to 0.
 *
 */
char *generator_cmd(parm_link_s *pl, const void *data, int level)
{
	const char *cmd_pre;
	static const cmd_s *cmd_p;

	PRE(data != NULL, pl != NULL);
	PRE((parm_type(pl) == tt_str) || (parm_type(pl) == tt_end));

	if (level == 0)
		cmd_p = (cmd_s *) data;

	/* Find fine command */
	cmd_pre = ((parm_type(pl) == tt_str) ? parm_str(pl) : "");
	while ((cmd_p->name) && (!cmd_lprefix(cmd_pre, cmd_p)))
		cmd_p++;

	return (cmd_p->name ? dup_lname((cmd_p++)->name) : NULL);
}


/** Safely add a parameter name
 *
 * Append the par string to the dest string ovewritting the '\0' character
 * at the end of the dest. A maximum dest size is checked thus no characters
 * above max_char are stored. The par string ends witch '\0' or '/'.
 *
 * @param dest     A string to be enlarged.
 * @param par      A parameter string.
 * @param max_char The maximum of characters dest can have.
 *
 */
static void cat_parm(char *dest, const char *par, size_t max_char)
{
	PRE(dest != NULL, par != NULL);

	size_t t = strlen(dest);
	size_t i;
	
	for (i = 0; (par[i]) && (par[i] != '/') && (t < max_char); i++, t++)
		dest[t] = par[i];
	
	dest[t] = '\0';
}


/** Print a help text
 *
 * Print a help on the screen generated from the command structure.
 *
 * @param cmds commands
 *
 */
void cmd_print_help(const cmd_s *cmds)
{
	const char *s;

	PRE(cmds != NULL);

	cmds++;
	
	mprintf("Command & arguments  Description\n");
	mprintf("-------------------- -------------------->\n");

	while (cmds->name) {
		int opt = 0;
		char spars[SNAME_SIZE];

		snprintf(spars, SNAME_SIZE, "%s", cmds->name);

		s = cmds->pars;
		while ((*s != ENDC) && (*s != CONTC)) {
			/* Append a space */
			cat_parm(spars, " ", SNAME_SIZE);

			/* Check optional */
			if (*s == OPTC) {
				cat_parm(spars, "[", SNAME_SIZE);
				opt++;
			}

			/* Copy short parameter name */
			cat_parm(spars, s + 2, SNAME_SIZE);

			s = find_next_parm(s);
		}
		
		/* Close brackets */
		for (; opt; opt--)
			cat_parm(spars, "]", SNAME_SIZE);
		
		mprintf("%-20s %s\n", spars, cmds->desc);
		
		cmds++;
	}
}


/** Print a help text about a command.
 *
 * @param parm Parameters - maximum is one string
 * @param cmds The array of supported commands
 *
 */
void cmd_print_extended_help(parm_link_s *parm, const cmd_s *cmds)
{
	const char *const cmd_name = parm_str(parm);
	const cmd_s *cmd;
	const char *s;
	int opt, par;

	PRE(parm != NULL, cmds != NULL);
	
	if (parm_type(parm) == tt_end) {
		cmd_print_help(cmds);
		return;
	}
	
	/* Find fine command */
	switch (cmd_find(cmd_name, cmds, &cmd)) {
	case CMP_NH:
		intr_error("Unknown command: %s", cmd_name);
		return;
	case CMP_MHIT:
		intr_error("Ambiguous command: %s", cmd_name);
		return;
	}
	
	mprintf("%s\n\n", cmd->descf);
	
	/* Print parameters */
	opt = 0;
	par = 0;
	s = cmd->pars;
	if ((*s != ENDC) && (*s != CONTC)) {
		mprintf("Syntax: %s", cmd->name);
		
		while ((*s != ENDC) && (*s != CONTC)) {
			/* Append a space */
			mprintf(" ");
			
			/* Check optional */
			if (*s == OPTC) {
				mprintf( "[");
				opt++;
			}
			
			/* Print short parameter name */
			if (*(s + 1) != CONC) {
				char *p = safe_strdup(s + 2);
				p[find_sname_len(s)] = 0;
				
				mprintf("<");
				mprintf("%s", p);
				mprintf(">");
				
				safe_free(p);
			} else 
				mprintf("%s", parm_skipq(s));
			
			par++;
			s = find_next_parm(s);
		}
		
		/* Close brackets */
		for (; opt; opt--)
			mprintf("]");
		
		if (par) {
			mprintf("\n\n");
			
			for (s = cmd->pars; (*s != ENDC) && (*s != CONTC); s = find_next_parm(s))
				if (*(s + 1) != CONC) {
					char buf[64];
					
					/* Print long parameter description */
					snprintf(buf, find_sname_len(s) + 1, "%s", parm_skipq(s));
					mprintf("\t<%s> %s\n", buf, find_lname(s));
				}
		} else
			mprintf( "\n");
	}
}
