/*
 * Command line parser
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifndef _PARSER_H_
#define _PARSER_H_


#define PARSER_MAX_STR	256

#include "mtypes.h"


// Line number
extern int lineno;

/* careful
 * note to add overview to token_overview
 */
enum token_enum
{
	tt_end,
	tt_int,
	tt_str,
	tt_err,
	tt_err_string_too_long,
	tt_err_string_noend,
	tt_err_invalid_num,
	tt_err_invalid_hex_num,
	tt_err_overflow
};


struct token_s
{
	enum token_enum ttype;

	union {
		uint32_t i;
		char *s;
	} tval;
};
typedef struct token_s token_s;


/*
 * Command parameter types
 *
 * Parameters of a command are represented as a collection strings. Each
 * string describes one parameter, strings are separated by a null character.
 * 
 * A parameter description consists of two flags and a name. The fisrt flag
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

#define REQ	"r"	/* parameter is required */
#define REQC	'r'
#define OPT	"o"	/* parameter is optional */
#define OPTC	'o'

#define INT	"i"	/* integer */
#define INTC	'i'
#define STR	"s"	/* string */
#define STRC	's'
#define VAR	"v"	/* any */
#define VARC	'v'
#define CON	"c"	/* constant required */
#define CONC	'c'

#define NEXT	"\0"	/* mark the next parameter */

#define NOCMD	"e"	/* no command */
#define NOCMDC	'e'
#define CONT	"\0n"	/* do not check other params */
#define CONTC	'n'
#define END	"\0e"	/* no more parameter */
#define ENDC	'e'

#define DEFAULT	NULL


/*
 * cmd_find return values
 */
#define CMP_NH		0	/* no hit */
#define CMP_HIT		1	/* hit */
#define CMP_PHIT	2	/* partial hit */
#define CMP_MHIT	3	/* multi-hit */

struct parm_link_s
{
	token_s token;
	
	struct parm_link_s *next;
};
typedef struct parm_link_s parm_link_s;

extern parm_link_s pars_end;

extern const char *token_overview[];

struct cmd_s;
typedef struct cmd_s cmd_s;

typedef bool (*cmd_f)( parm_link_s *parm, void *data);
typedef char *(*gen_f)( parm_link_s *pl, const void *data,
		int level);
typedef void (*fgen_f)( parm_link_s **pl, const cmd_s *cmd,
		gen_f *generator, const void **data);


/*
 * Device command list
 */
struct cmd_s
{
	const char *name;	/* command name */
	cmd_f func;		/* function which implements command */
	fgen_f find_gen;	/* finds the completion function */
	cmd_f help;		/* displays help */
	const char *desc;	/* short description */
	const char *descf;	/* full description */
	const char *pars;	/* parameters and description */
};

bool hexadecimal( char c);
int hex2int( char c);
bool numeric_hex_char( char c);

parm_link_s *parm_parse( const char *s);
void parm_delete( parm_link_s *pl);
void parm_set_str( parm_link_s *pl, const char *s);

int cmd_find( const char *cmd_name, const cmd_s *cmds,
		const cmd_s **cmd);
parm_link_s *parm_next( parm_link_s **pl);
void parm_check_end( parm_link_s *pl, const char *input);

int parm_type( parm_link_s *parm);
uint32_t parm_int( parm_link_s *parm);
char *parm_str( parm_link_s *parm);
uint32_t parm_next_int( parm_link_s **parm);
char *parm_next_str( parm_link_s **parm);

bool parm_insert_int( parm_link_s *parm, uint32_t val);
bool parm_insert_str( parm_link_s *parm, char *s);

void parm_change_int( parm_link_s *parm, uint32_t val);

bool cmd_run_by_spec( const cmd_s *cmd, parm_link_s *parm, void *data);
bool cmd_run_by_name( const char *cmd, parm_link_s *parm, const cmd_s *cmds,
		void *data);
bool cmd_run_by_parm( parm_link_s *pl, const cmd_s *cmds, void *data);

char *generator_cmd( parm_link_s *pl, const void *data, int level);

void cmd_print_help( const cmd_s *cmds);
void cmd_print_extended_help( parm_link_s *parm,
		const cmd_s *cmds);

#endif /* _PARSER_H_ */
