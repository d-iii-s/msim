/*
 * Parsing setup file
 *
 * Copyright (c) 2001-2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

#include "mtypes.h"
#include "output.h"
#include "check.h"
#include "utils.h"
#include "device.h"
#include "cline.h"


#define SNAME_SIZE	32


struct g_token_s
{
	enum token_enum ttype;

	uint32_t i;
	char s[ PARSER_MAX_STR];
};
typedef struct g_token_s g_token_s;


const char *token_overview[] =
{
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
	{ tt_end},
	NULL
};


/** multiply_char - Returns the multiplication constant.
 *
 * The user can specify the multiplication constant. It is a character
 * following the number.
 * 
 * SE Moves the pointer to the next character if matches.
 */
static int
multiply_char( char c)

{
	switch (c)
	{
		case 'k': return 1024;
		case 'K': return 1000;
		case 'M': return 1024*1024;
	}
	return 1;
}


/** white - Tests whether the character is white.
 * 
 * White chars are spaces, tabs and comments.
 */
static bool
white( char c)

{
	return c == ' ' || c == '\t';
}


/** decimal -- Tests whether the character is a deciamal.
 *
 * R	true if the character is within 0..9
 */
static bool
decimal( char c)

{
	return c >= '0' && c <= '9';
}


/** hexadecimal - A "hex number" test.
 *
 * R	true if the character is within 0..9, a..f
 */
bool
hexadecimal( char c)

{
	return (c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F');
}


/** alphanum - A "character or number" test.
 *
 *  R		true if the character is within 0..9, a..z or _
 */
static bool
alphanum( char c)

{
	return (c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c == '_');
}


/* skip_white_chars - Moves the pointer to the next relevant character.
 */
static void
skip_white_chars( const char **s)

{
	const char *c = *s;
	
	PRE( s != NULL);
	PRE( *s != NULL);

	do {
		// skip white chars
		while (*c && white( *c))
			c++;

		// skip comments
		if (*c == '#')
		{
			while (*c && (*c != '\n'))
				c++;
			continue;
		}
	} while (false);

	*s = c;
}




/** hex2int -- A character to integer conversion.
 *
 * R	a value of the character
 * 	-1 if error
 */
int
hex2int( char c)

{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'z')
		return c -'a' +10;
	if (c >= 'A' && c <= 'Z')
		return c -'A' +10;
	return -1;
}


/** read_number - Converts the text number into the integer.
 */
void
read_number( const char **s, g_token_s *t)

{
	const char *c = *s;
	uint32_t i;
	volatile uint32_t oi, oi2;

	PRE( s != NULL, t != NULL);
	PRE( *s != NULL);

	oi = 0;
	i = 0;
	
	if (*c=='0' && *(c+1)=='x')
	{
		/* hex number */
		c += 2;
		if (!hexadecimal( *c))
		{
			t->ttype = tt_err_invalid_hex_num;
			return;
		}

		for (; (oi<=i) && hexadecimal( *c);c++)
	
		{
			oi = i;
			i = i*16 +hex2int( *c);
		}
	}
	else
	{
		/* dec number */
		for (; oi<=i && decimal( *c); c++)
		
		{
			oi = i;
			i = i*10 +(*c -'0');
		}
	}
	
	/* multiply */
	oi2 = i;
	i *= multiply_char( *c);
	c += oi2!=i;
	
	/* overflow test */
	if ((oi>i) || (oi2>i))
	{
		t->ttype = tt_err_overflow;
		return;
	}

	t->i = i;
	*s = c;

	/* error test */
	if (*c && alphanum( *c))
		t->ttype = tt_err_invalid_num;
	else
		t->ttype = tt_int;
}


/** read_string - Reads a string token.
 *
 * The size limit of the string token is specified by the PARSER_MAX_STR
 * constant.
 *
 * IN s		pointer to the input token
 * OUT t	a token structure to store the string
 */
static void
read_string( const char **s, g_token_s *t)

{
	int i2 = 0;
	const char *c = *s;
	char *c2 = t->s;
	char cx;

	PRE( s != NULL, t != NULL);
	PRE( *s != NULL);

	if ((*c == '"') || (*c == '\''))
	
	{
		cx = *c;
		
		/* test for a "" */
		if (*(++c) == cx) c++;
		else
		{
			/* reading hard string */
			while (*c && (*c!=cx) && (*c!='\n') &&
					(i2<PARSER_MAX_STR-1)) 
				{ *c2 = *c; c++; c2++; i2++; }

			/* eol test */
			if (*c == '\n')
			{
				t->ttype = tt_err_string_noend;
				return;
			}

			/* length test */
			if (i2 == PARSER_MAX_STR-1)
			{
				t->ttype = tt_err_string_too_long;
				return;
			}

			c++;
		}
	}
	else /* special character test */
	if (!alphanum( *c))
	{
		*c2 = *c; c++; c2++;
	}
	else 
	{
			
		/* reading string to the next non-string character */
		while ((alphanum( *c)) && (i2<PARSER_MAX_STR-1))
			{ *c2 = *c; c++; c2++; i2++; }

		/* length test */
		if (i2 == PARSER_MAX_STR-1)
		{
			t->ttype = tt_err_string_too_long;
			return;
		}
	}
	
	*s = c;
	*c2 = 0;
	t->ttype = tt_str;
}


/** read_token - Reads the next token.
 *
 * IN s		pointer to the current position
 * OUT t	a structure to store the next token
 */
static void
read_token( const char **s, g_token_s *t)

{
	const char *c = *s;

	PRE( s != NULL, t != NULL);
	PRE( *s != NULL);
	
	/* end ? */
	if ((*c == 0) || (*c == '\n'))
	{
		t->ttype = tt_end;
		(*s)++;
	}
	else
	/* number ? */
	if (decimal( *c))
		read_number( s, t);
	/* string ! */ 
	else read_string( s, t);
}


/** parse_g_next - Uploads next token.
 *
 * SE The position pointer is moved to the character next to the token.
 *
 * IO s		The pointer to the current position
 * OUT t	Output token
 */
void
parse_g_next( const char **s, g_token_s *t)

{
	PRE( s != NULL, t != NULL);
	PRE( *s != NULL);

	skip_white_chars( s);
	read_token( s, t);
}


/** parse_next - Uploads next token (exretnal version).
 * 
 * The external token is different from the internal parser token -
 * the external string tokens are stored in the heap and have
 * to be disposed by the callie.
 *
 * SE The position pointer is moved to the character next to the token.
 *
 * IO s		a pointer to the current position
 * OUT t	a token structure to store the next token
 */
void
parse_next( const char **s, token_s *t)

{
	g_token_s gt;

	PRE( s != NULL, t != NULL);
	PRE( *s != NULL);
	
	parse_g_next( s, &gt);

	t->ttype = gt.ttype;
	switch (gt.ttype)
	{
		case tt_int:
			t->tval.i = gt.i;
			break;
		case tt_str:
			t->tval.s = xstrdup( gt.s);
			break;
		default:
			/* nothing else */
			break;
	}
}


/** strndup -- Make a string duplicate up to the speficied size.
 *
 * It is similar to the GNU extension variant.
 */
char *
strndup( const char *s, size_t max)

{
	int len;
	char *r;

	PRE( s != NULL);

	for (len=0; s[ len] && len<max; len++) ;
	
	r = xmalloc( len+1);
	memcpy( r, s, len);
	r[ len] = '\0';

	return r;
}


/** parm_delete - Deletes the parameter list.
 *
 * Also all internal data like strings are freed.
 *
 * IN pl	parm structure, may be NULL (no operation is performed)
 */
void
parm_delete( parm_link_s *pl)

{
	parm_link_s *p;

	for (p=pl; p; p=pl)

	{
		pl = p->next;

		if (p->token.ttype == tt_str)
			free( p->token.tval.s);
		free( p);
	}
}


/** parm_parse - Parses the input line and links all token into the parm list.
 *
 * R	A pointer to the first element, otherwise NULL as a not-enough-memory
 * 	error.
 */
parm_link_s *
parm_parse( const char *s)

{
	token_s *t;
	parm_link_s *pl, **p = &pl;
	g_token_s gt;

	PRE( s != NULL);
	
	do {
		/* parse next token */
		parse_g_next( &s, &gt);
		
		/* allocate a new node */
		*p = (parm_link_s *)
			xmalloc( sizeof( parm_link_s));
		
		/* copy parameters */
		t = &(*p)->token;
		t->ttype = gt.ttype;
		switch (t->ttype)
		{
			case tt_int:
				t->tval.i = gt.i;
				break;
			case tt_str:
				t->tval.s = xstrdup( gt.s);
				if (!t->tval.s)
				{
					/* memory allocation error */
					t->ttype = tt_err;
					parm_delete( pl);
					return NULL;
				}
				break;
			default:
				break;
		}
		(*p)->next = NULL;
		
		p = &(*p)->next;
	} while (gt.ttype != tt_end && gt.ttype < tt_err);
	
	return pl;
}


/** Checks for the end of the string.
 *
 * If there is a space between the end of the input string and the last
 * character, the separator is included into the parameter link.
 */
void parm_check_end( parm_link_s *pl, const char *input)

{
	parm_link_s *plo;
	size_t sl;

	PRE( pl != NULL);
	
	if (parm_type( pl) == tt_end)
		return;
	
	sl = strlen( input);

	if (sl && white( input[ sl-1]))

	{
		// find the end of the parameter list
		plo = pl;
		while (parm_type( parm_next( &pl)) != tt_end)
			plo = pl;

		parm_insert_str( plo, xstrdup( ""));
	}
}


/** find_lname - Finds a long name.
 * 
 * Function searchs for a long parameter name and returns its first character.
 * When no semarator is founded the whole string is a long name.
 *
 * IN s		Input string. The format is "short_name/long_name".
 */
const char *
find_lname( const char *s)

{
	const char *s2;

	PRE( s != NULL);
	
	/* seach for a separator */
	s += 2;
	for (s2=s; *s2 && *s2 != '/'; s2++) ;
	
	return (*s2) ? s2+1 : s;
}


/** find_sname_len - Finds a short name.
 * 
 * Function searchs for a short parameter name and returns its length.
 * When no semarator is founded the whole string is a short name.
 *
 * IN s		Input string. The format is "short_name/long_name".
 * R		Size of the name.
 */
int
find_sname_len( const char *s)

{
	const char *s2;

	PRE( s != NULL);
	
	/* seach for a separator */
	s += 2;
	for (s2=s; s2 && *s2 != '/'; s2++) ;
	
	return s2-s;
}


/** find_next_parm - Finds next parameter.
 * 
 * Returns a pointer to the first character following a first terminal.
 *
 * IO s		The input string. The format is "text\0another text".
 */
const char *
find_next_parm( const char *s)

{
	PRE( s != NULL);

	while (*s++) ;
	return s;
}


/** dup_lname - Allocs a new string containing the long name.
 *
 * IN lname	A string of format "long_name/short_name" or just only
 * 		the long string.
 */
char *
dup_lname( const char *lname)

{
	int len;

	PRE( lname);
	PRE( *lname != '\0', *lname != '/');

	for (len=0; lname[ len] && lname[ len] != '/'; len++) ;

	return strndup( lname, len);
}


/** cmdcmp - Compares the command with a name definition.
 *
 * Compares the input command with the command repository.
 * IO s		the user input
 * IO cmd	supproted commands. Command names/variants are separated via
 * 		the '/' character.
 * R		CMP_NH - no hit
 * 		CMP_PHIT - partial hit, the specified command is a prefix of another command
 *		CMP_HIT - full hit
 */
int
cmdcmp( const char *s, const char *cmd)

{
	int phit = 0;
	const char *s2;

	PRE( s != NULL, cmd != NULL);

	do {
		if (*cmd == '/')
			cmd++;
		
		/* compare strings */
		for (s2=s; *cmd && *cmd != '/' && *cmd == *s2; cmd++, s2++) ;
		
		/* check full hit */
		if (!*s2)
		{
			if (!*cmd || *cmd == '/')
				return CMP_HIT;
			phit++;
		}
		
		/* go to the next command variation */
		while (*cmd && *cmd != '/')
			cmd++;
	} while (*cmd);

	return phit ? CMP_PHIT : CMP_NH;
}


/** cmd_lprefix - Compares the parial name with the long command name.
 *
 */
bool
cmd_lprefix( const char *par_name, const cmd_s *cmd)

{
	const char *s;
	const char *s2;

	PRE( par_name != NULL, cmd != NULL);
	PRE( *par_name != '/');

	// compare strings
	s = par_name;
	s2 = cmd->name;
	while (*s && *s2 != '/' && *s == *s2)
	{
		s++;
		s2++;
	}

	return *s2 == '/' || *s == '\0';
}


/** parm_next - Moves the pointer to the next parameter.
 */
parm_link_s *
parm_next( parm_link_s **pl)

{
	PRE( pl != NULL);
	PRE( *pl != NULL);

	return *pl = (*pl)->next;
}


/** parm_type - Returns the parameter type.
 */
int
parm_type( parm_link_s *parm)

{
	PRE( parm != NULL);
	return parm->token.ttype;
}


/** parm_init - Returns the integer parameter.
 */
uint32_t
parm_int( parm_link_s *parm)

{
	PRE( parm != NULL);
	return parm->token.tval.i;
}


/** parm_str - Returns the string parameter.
 */
char *
parm_str( parm_link_s *parm)

{
	PRE( parm != NULL);
	return parm->token.tval.s;
}


/** parm_next_int - Returns the integer parameter and move to the next one.
 */
uint32_t
parm_next_int( parm_link_s **parm)

{
	uint32_t u;

	PRE( parm != NULL); PRE( *parm != NULL);
	
	u = (*parm)->token.tval.i;
	*parm = (*parm)->next;

	return u;
}


/** parm_next_str - Returns the integer parameter and moves to the next one.
 */
char *
parm_next_str( parm_link_s **parm)

{
	char *s;

	PRE( parm != NULL); PRE( *parm != NULL);

	s = (*parm)->token.tval.s;
	*parm = (*parm)->next;

	return s;
}


/** parm_insert_int - Inserts an integer parameter.
 *
 * A new parameter is inserted NEXT TO the specified one.
 */
bool
parm_insert_int( parm_link_s *pl, uint32_t val)

{
	parm_link_s *p;

	PRE( pl != NULL);

	p = xmalloc( sizeof( token_s));

	p->token.ttype = tt_int;
	p->token.tval.i = val;
	p->next = pl->next;
	pl->next = p;

	return true;
}


/** parm_insert_str - Inserts a string parameter into the parameter list.
 *
 * A new parameter is inserted NEXT TO the specified one.
 */
bool
parm_insert_str( parm_link_s *pl, char *s)

{
	parm_link_s *p;
	
	PRE( pl != NULL, s != NULL);

	p = xmalloc( sizeof( token_s));

	p->token.ttype = tt_str;
	p->token.tval.s = s;
	p->next = pl->next;
	pl->next = p;
	
	return true;
}


/** parm_change_int - Changes the parameter type to int.
 */
void
parm_change_int( parm_link_s *parm, uint32_t val)

{
	PRE( parm != NULL);

	if (parm->token.ttype == tt_str)
		free( parm->token.tval.s);
	parm->token.ttype = tt_int;
	parm->token.tval.i = val;
}


/** parm_set_str - Sets a token to a string.
 *
 * Function sets the item of a parameter list to specify a string token.
 */
void
parm_set_str( parm_link_s *pl, const char *s)

{
	PRE( pl != NULL, s != NULL);

	pl->token.ttype = tt_str;
	strcpy( pl->token.tval.s, s);
}


/** parm_skipq - Skips qualifiers from a parameter description.
 */
const char *
parm_skipq( const char *s)

{
	PRE( s != NULL);

	return s+2;
}


/** cmd_find - Checks the command name.
 *
 * Function check the command name against command description structure.
 * Returns apropriate value CMD_* and a pointer to the founded command.
 *
 * IN cmd_name	name of the command
 * IN cmds	command structures
 * IN cmd	output - the founded command, NULL means no output
 */
int
cmd_find( const char *cmd_name, const cmd_s *cmds,
		const cmd_s **cmd)

{
	int phit;

	PRE( cmd_name != NULL, cmds != NULL);

	/* find fine command */
	for (phit = 0; phit != -1 && cmds->name; cmds++)
		switch (cmdcmp( cmd_name, cmds->name))
		{
			case CMP_NH:
				break;
			case CMP_PHIT:
				if (phit++ == 0 && cmd)
					*cmd = cmds;
				break;
			case CMP_HIT:
				phit = -1;
				if (cmd)
					*cmd = cmds;
				break;
		}
	
	/* count output */
	switch (phit)
	{
		case -1:
			return CMP_HIT;
		case 0:
			return CMP_NH;
		case 1:
			return CMP_HIT;
	}

	return CMP_MHIT;
}


/** cmd_run_by_spec - Executes the command passed as the pointer to
 * 		the command structure.
 */
bool
cmd_run_by_spec( const cmd_s *cmd, parm_link_s *parm,
		void *data)

{
	const char *s;
	parm_link_s *p = parm;

	PRE( cmd != NULL, parm != NULL);

	/*
	 * Now we have to go over all parameters and check them.
	 */
	s = cmd->pars;
	while (*s != CONTC && *s != ENDC)
	{
		/* check the first quantifier */
		if (*s != OPTC && *s != REQC)
		{
			dprintf_err( "Internal error: invalid parameter quantifier: \"%s\".\n",
					s);
			return false;
		}
		
		if (*s == OPTC && p->token.ttype == tt_end)
			break;
		else
		if (*s == REQC && p->token.ttype == tt_end)
		{
			dprintf_err( "Missing parameter <%s>\n",
					find_lname( s));

			return false;
		}

		/* check type */
		switch (*(s+1))
		{
			case INTC:
				if (p->token.ttype != tt_int)
				{
					dprintf_err( "Invalid argument, integer <%s> required.\n",
							find_lname( s));
					return false;
				}
				break;
			case STRC:
				if (p->token.ttype != tt_str)
				{
					dprintf_err( "Invalid argument, string <%s> required.\n",
							find_lname( s));
					return false;
				}
				break;
			case VARC:
				if (p->token.ttype != tt_int && p->token.ttype != tt_str)
				{
					dprintf_err( "Invalid argument, string or integer <%s> required.\n",
							find_lname( s));
					return false;
				}
				break;
			case CONC:
				if (p->token.ttype != tt_str ||
					strcmp( parm_skipq( s), p->token.tval.s))
				{
					dprintf_err( "Invalid argument, string \"%s\" required.\n",
							find_lname( s));
					return false;
				}
				break;
			default:
				dprintf_err( "Internall error: Invalid parameter type.\n");
				return false;
		}
		
		s = find_next_parm( s);		/* next parameter */
		parm_next( &p);			/* next token */
	}

	if (*s == ENDC && p->token.ttype != tt_end)
	{
		dprintf_err( "Too many parameters.\n");
		return false;
	}

		
	/*
	 * Check whether the specified function is already implemented.
	 * If so, call it.
	 */
	if (!cmd->func)
	{
		intr_error( "Command not implemented.\n");
		return false;
	}
	
	return cmd->func( parm, data); 
}


/** cmd_run_by_name - Executes the specified command.
 * 
 * The command specified by the parameter is searched in the command structure,
 * parameters are verified and if successful, the command si executed.
 */
bool
cmd_run_by_name( const char *cmd_name, parm_link_s *parm,
		const cmd_s *cmds, void *data)

{
	const cmd_s *cmd;

	PRE( cmd_name != NULL, parm != NULL, cmds != NULL);

	/* find fine command */
	switch (cmd_find( cmd_name, cmds, &cmd))
	{
		case CMP_NH:
			intr_error( "Unknown command: %s", cmd_name);
			return false;
		case CMP_MHIT:
			intr_error( "Ambiguous command: %s", cmd_name);
			return false;
	}
	
	return cmd_run_by_spec( cmd, parm, data);
}


/** Applies the specified command.
 * 
 * The command is specified by the first parameter.
 */
bool
cmd_run_by_parm( parm_link_s *pl, const cmd_s *cmds,
		void *data)

{
	PRE( pl != NULL, cmds != NULL);

	/* check whether the first token is a string */
	if (pl->token.ttype != tt_str)
	{
		intr_error( "Command name expected.");
		return false;
	}
	
	return cmd_run_by_name( pl->token.tval.s, pl->next, cmds, data);
}


/**
 * 
 * IN level	The call number. It is only necessary that the first call has
 * 		a number 0.
 * IN pl	type tt_str
 */
char *
generator_cmd( parm_link_s *pl, const void *data, int level)

{
	const char *cmd_pre;
	static const cmd_s *cmd_p;

	PRE( data != NULL, pl != NULL);
	PRE( parm_type( pl) == tt_str || parm_type( pl) == tt_end);

	if (level == 0)
		cmd_p = (cmd_s *)data;

	/* find fine command */
	cmd_pre = parm_type( pl) == tt_str ? parm_str( pl) : "";
	while (cmd_p->name &&
			!cmd_lprefix( cmd_pre, cmd_p))
		cmd_p++;

	return cmd_p->name ? dup_lname( (cmd_p++)->name) : NULL;
}


/** cat_parm - Safe adds a parameter name.
 *
 * Appends the par string to the dest string ovewritting the '\0' character
 * at the end of the dest. A maximum dest size is checked thus no characters
 * above max_char are stored. The par string ends witch '\0' or '/'.
 *
 * IO dest	A string to be enlarged.
 * IN par	A parameter string.
 * IN max_char	The maximum of characters dest can have.
 */
void
cat_parm( char *dest, const char *par, int max_char)

{
	size_t t;
	int i;

	PRE( dest != NULL, par != NULL);

	t = strlen( dest);
	
	for (i=0; par[ i] && par[ i] != '/' && t<max_char; i++, t++)
		dest[ t] = par[ i];
	
	dest[ t] = '\0';
}


/** cmd_printhelp - Prints a help text.
 *
 * Prints a help on the screen generated from the command structure.
 *
 * IN cmds	commands
 */
void
cmd_print_help( const cmd_s *cmds)

{
	const char *s;

	PRE( cmds != NULL);

	cmds++;

	dprintf( "List of available commands:\n");

	while (cmds->name)
	{
		int opt = 0;
		char spars[ SNAME_SIZE];

		snprintf( spars, SNAME_SIZE, "%s", cmds->name);

		s = cmds->pars;
		while (*s != ENDC && *s != CONTC)
		{
			/* append a space */
			cat_parm( spars, " ", SNAME_SIZE);

			/* check optional */
			if (*s == OPTC)
			{
				cat_parm( spars, "[", SNAME_SIZE);
				opt++;
			}

			/* copy short parameter name */
			cat_parm( spars, s+2, SNAME_SIZE);

			s = find_next_parm( s);
		}
		
		/* close brackets */
		for (; opt; opt--)
			cat_parm( spars, "]", SNAME_SIZE);
		
		dprintf( "%-20s %s\n", spars, cmds->desc);
		
		cmds++;
	}
}


/** cmd_print_extended_help - Prints a help text about a command.
 *
 * IN parm	parameters - maximum is one string.
 * IN cmds	the array of supported commands
 */
void
cmd_print_extended_help( parm_link_s *parm,
		const cmd_s *cmds)

{
	const char *const cmd_name = parm_str( parm);
	const cmd_s *cmd;
	const char *s;
	int opt, par;

	PRE( parm != NULL, cmds != NULL);
	
	if (parm_type( parm) == tt_end)
	{
		cmd_print_help( cmds);
		return;
	}
	
	/* find fine command */
	switch (cmd_find( cmd_name, cmds, &cmd))
	{
		case CMP_NH:
			intr_error( "Unknown command: %s", cmd_name);
			return;
		case CMP_MHIT:
			intr_error( "Ambiguous command: %s", cmd_name);
			return;
	}
	
	dprintf_text( "%s", cmd->descf);
	
	/* print parameters */
	opt = 0;
	par = 0;
	s = cmd->pars;
	if (*s != ENDC && *s != CONTC)
	{
		dprintf( "Syntax: %s", cmd->name);
		
		while (*s != ENDC && *s != CONTC)
		{
			/* append a space */
			dprintf( " ");
			
			/* check optional */
			if (*s == OPTC)
			{
				dprintf( "[");
				opt++;
			}
			
			/* print short parameter name */
			if (*(s+1) != CONC)
			{
				dprintf( "<");
				dprintf_n( find_sname_len( s), "%s", s+2);
				dprintf( ">");
			}
			else
				dprintf( "%s", parm_skipq( s));
			
			par++;
			s = find_next_parm( s);
		}
		
		/* close brackets */
		for (; opt; opt--)
			dprintf( "]");
		
		if (par)
		{
			dprintf( "\nwhere:\n");
			
			for (s = cmd->pars; *s != ENDC && *s != CONTC; s = find_next_parm( s))
				if (*(s+1) != CONC)
				{
					char buf[ 64];
					
					/* print long parameter description */
					snprintf( buf, find_sname_len( s)+1, "%s", parm_skipq( s));
					dprintf( "\t<%s> %s\n", buf, find_lname( s));
				}
		}
	}
}
