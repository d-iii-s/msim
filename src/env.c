/*
 * Environment veriables
 * 
 * Copyright (c) 2002-2004 Viliam Holub 
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "env.h"

#include "mtypes.h"
#include "parser.h"
#include "output.h"
#include "instr.h"
#include "check.h"
#include "utils.h"


/*
 *
 * Variables
 *
 */

/*
 * Instruction disassembling
 */

bool iaddr	= true;
bool iopc	= false;
bool icmt	= true;
bool iregch	= true;
int ireg	= 2;

/*
 * Debugging
 */
bool totrace	= false;


char **regname;


/*
 * Boolean constants
 */
const char *t_bool[] =
	{ "on", "true", "yes", "off", "false", "no", NULL};
const char *t_true_all[] =
	{ "on", "t", "tr", "tru", "true", "y", "ye", "yes", NULL};
const char *t_false_all[] =
	{ "off", "f", "fa", "fal", "fals", "false", "n", "no", NULL};




/*
 * The set_struct structure contains a description of simulator environment
 * variables.
 *
 * name	Is the name of a variable as it is visible to the user.
 * desc	Is a short description text. It is displayed as the help text to
 * 	the variable.
 * descf Is a full description of the variable. Displayed when user asks
 * 	for help about that variable.
 * type	A variable type - bool, int, str etc.
 * val	A pointer where to store the value
 * func	A function which is called to change a variable
 *
 * name == NULL means no other variables
 * name != NULL && val == NULL means this is not a real variable but
 * 	a separator which is used to display a more user-friendly help
 * 	output
 * func != NULL means that a func function have to be called instead direct
 * 	write to the variable
 */
struct set_s
{
	const char *const name;		/* name of the variable */
	const char *const desc;		/* brief textual description */
	const char *const descf;	/* full textual description */
	var_type_e type;		/* variable type */
	void *val;			/* where to store a value */
	void *const func;		/* function to be called */
};
typedef struct set_s set_s;


typedef bool (*set_int_f)( int);
typedef bool (*set_bool_f)( bool);
typedef bool (*set_str_f)( const char *);


static bool change_ireg( int i);


/*
 * Description of variables
 */

const set_s env_set[] =
{
{ "disassembling",
		"Disassembling features"},
	
	{ "iaddr",
		"Set whether to display instruction addresses.",
		"The iaddr variable sets displaying address of each disassembled "
			"instruction. This feature is useful especially together with the trace "
			"variable.",
		vt_bool,
		&iaddr,
		NULL},
	{ "iopc",
		"Set when instruction opcodes should be displayed.",
		"Set this variable to show instruction opcodes. Althrow an instruction "
			"opcode is not a human friendly representation, there exists"
			"reasons when the opcode knowledge may help (debugging random write "
			"accesses for example).",
		vt_bool,
		&iopc,
		NULL},
	{ "icmt",
		"Allows commants for instructions.",
		"Set this variable to show information about the disassembled instruction. "
			"Currenty this is the hex to decimal parameter conversion.",
		vt_bool,
		&icmt,
		NULL},
	{ "iregch",
		"Set whether to display register changes.",
		"This is a debugging feature - registers which has been modified during "
			"instruction execution are displayed together with a previous and a new "
			"value.",
		vt_bool,
		&iregch,
		NULL},
	{ "ireg",
		"Set register name mode.",
		"There are several modes how register names could be displayed. The first one "
			"is technical - every register name consist of the 'r' prefix following "
			"the register number (example - r0, r12, r22, etc.). The second one "
			"is very similar, the prefix is a '$' mark which is used by the AT&T "
			"assembler. Finally there is a textual naming convention based on "
			"a register usage (at, t4, s2, etc.)." ,
		vt_int,
		&ireg,
		change_ireg},
	
{ "debugging",
		"Debugging features"},

	{ "trace",
		"Set disassembling of instructions as they are executed.",
		"Via the trace variable you may choose whether all instructions should "
			"be disassembled as they are executed.",
		vt_bool,
		&totrace,
		NULL},
	
	{}
};



/** change_ireg - Changes the ireg variable.
 *
 * R		true if successful
 */
static bool
change_ireg( int i)

{
	if (i > 2)
	{
		mprintf( "Index out of range 0..2.\n");
		return false;
	}
	
	ireg = i;
	regname = RegName[ i];
	
	return true;
}


/** Prints all variables and its states.
 */
static void
print_all_variables( void)

{
	const set_s *s = env_set;

	mprintf( "List of all variables:\n");
	
	while (s->name)
	{
		if (s->val)
		{
			/* variable */
			mprintf( "\t%s = ", s->name);
			switch (s->type)
			{
				case vt_int:
					mprintf( "%d", *(int *)s->val);
					break;
				case vt_str:
					mprintf( "%s", *(const char *)s->val);
					break;
				case vt_bool:
					mprintf( "%s", *( bool *)s->val ?
							"on" : "off");
					break;
			}
		}
		else
		{
			/* label */
			mprintf( "%s", s->desc);
		}
		mprintf( "\n");

		s++;
	}
}


/** Checks whether the variable exists.
 *
 */
bool
env_check_varname( const char *name, var_type_e *type)

{
	const set_s *s;
	bool b = false;

	if (!name)
		name = "";

	for (s = env_set; s->name; s++)
		if (s->val && !strcmp( name, s->name))
		{
			b = true;
			if (type)
				*type = s->type;
			break;
		}
		
	return b;
}


/** Checks whether the variable is boolean.
 *
 */
bool
env_bool_type( const char *name)

{
	const set_s *s;
	bool b = false;

	if (!name)
		name = "";

	for (s = env_set; s->name; s++)
		if (s->val && !strcmp( name, s->name))
		{
			b = s->type == vt_bool;
			break;
		}
		
	return b;
}


int
env_cnt_partial_varname( const char *name)

{
	const set_s *s;
	int cnt = 0;

	if (!name)
		name = "";

	for (s = env_set; s->name; s++)
		if (s->val && prefix( name, s->name))
			cnt++;
		
	return cnt;
}


bool
env_by_partial_varname( const char *name, const set_s **sx)

{
	const set_s *s = (sx && *sx) ? *sx+1 : env_set;

	if (!name)
		name = "";

	for (; s->name; s++)
		if (s->val && prefix( name, s->name))
			break;
		
	if (s->name && sx)
		*sx = s;

	return !!s->name;
}



/** Searches for the variable description.
 *
 * SE: Also prints an error message when the variable name was not found.
 *
 * IN  var_name	The variable name
 * R		Variable descripion
 */
static const set_s *
search_variable( const char *var_name)

{
	const set_s *s;
	
	for (s = env_set; s->name; s++)
		if (s->val && !strcmp( var_name, s->name))
			break;
	
	if (!s->name)
	{
		mprintf( "Unknown variable %s.\n", var_name);
		return NULL;
	}

	return s;
}


/** show_help  - Prints help text about variables.
 *
 * There are two types of help. The short one when the user typed "set help"
 * prints just all variables with a shor description. The long variant is
 * used when the user specify the variable name ("set help=var_name").
 *
 * IN  parm	Parameter points to the token following the "help" token.
 */
static void
show_help( parm_link_s *parm)

{
	const set_s *s;

	if (parm_type( parm) == tt_end)
		for (s = env_set; s->name; s++)
		{
			if (s->val)
				/* variable */
				mprintf( "\t%s  %s", s->name, s->desc);
			else
				/* label */
				mprintf( "%s", s->desc);
			mprintf( "\n");
		}
	else
	{
		s = search_variable( parm_str( parm->next));
		if (s)
		{
			mprintf( "%s\n", s->descf);
		}
	}
}


/** set_int - Sets an integer variable.
 */
static bool
set_int( const set_s *s, parm_link_s *parm)

{
	if (s->func)
		((set_int_f)s->func)( parm_int( parm));
	else
		*(uint32_t *)s->val = parm->token.tval.i;
		
	return true;
}


/** Sets a boolean variable.
 *
 */

static bool
set_bool( const set_s *s, parm_link_s *parm)

{
	parm->token.tval.i = !!parm->token.tval.i;
	
	if (s->func)
		((set_bool_f)s->func)( parm_int( parm));
	else
		*(bool *)s->val = !!parm->token.tval.i;
		
	return true;
}


/** set_str - Sets a string variable.
 */
static bool
set_str( const set_s *s, parm_link_s *parm)

{
	if (s->func)
		((set_str_f)s->func)( parm_str( parm));
	else
	{
		char *sx = xstrdup( parm->token.tval.s);
		if (!sx)
		{
			mprintf( "Not enough memory\n");
			return false;
		}
		free( s->val);
		*(char **)s->val = sx;
	}
		
	return true;
}


/** bool_sanitize - Converts a boolean string to integer.
 */
static bool
bool_sanitize( parm_link_s *parm)

{
	const char **s;

	if (parm->token.ttype == tt_str)
	{
		/* test for true */
		for (s=t_true_all; *s; s++)
			if (!strcmp( parm_str( parm), *s))
				break;
		if (*s)
		{
			parm_change_int( parm, 1);
			return true;
		}

		/* test for false */
		for (s=t_false_all; *s; s++)
			if (!strcmp( parm_str( parm), *s))
				break;
		if (*s)
		{
			parm_change_int( parm, 0);
			return true;
		}
	}

	return false;
}


/** set_variable - Sets variable.
 */
static bool
set_variable( parm_link_s *parm)

{
	const set_s *s = search_variable( parm_str( parm));
	
	if (!s)
		return false;

	parm_next( &parm);
	parm_next( &parm);

	switch (s->type)
	{
		case vt_int:
			return set_int( s, parm);
		case vt_bool:
			if (!bool_sanitize( parm))
			{
				mprintf( "Boolean parameter expected.\n");
				return false;
			}
			return set_bool( s, parm);
		case vt_str:
			return set_str( s, parm);
		default:
			return false;
	}
}


/** set_bool_variable - Decodes and sets or unsets a boolean variable.
 */
static bool
set_bool_variable( bool su, parm_link_s *parm)

{
	parm_link_s p = { {tt_int, {su}}, NULL};
	const set_s *s = search_variable( parm_str( parm));

	return s ? set_bool( s, &p) : false;
}


/** env_cmd_set - Set command implementation
 */
bool
env_cmd_set( parm_link_s *pl)

{
	if (parm_type( pl) == tt_end)
	{
		/* show all variables */
		
		print_all_variables();
		return true;
	}

	if (!strcmp( parm_str( pl), "help"))
	{
		/* set help */
		
		show_help( pl->next);
		return true;
	}
		
	/* implicit boolean? */
	if (parm_type( pl->next) == tt_end)
		return set_bool_variable( true, pl);

	/* set the variable */
	return set_variable( pl);
}


/** env_cmd_unset - Unset command implementation.
 */
bool
env_cmd_unset( parm_link_s *pl)

{
	return set_bool_variable( false, pl);
}


/*
 * TAB completion
 *
 * 
 */



/** Generates a list of environment variables.
 */
char *
generator_env_name( parm_link_s *pl, const void *data, int level)

{
	bool b;
	static const set_s *d;

	PRE( pl != NULL);
	PRE( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	// iterator initialization
	if (level == 0)
		d = NULL;
	
	// find next suitable entry
	b = env_by_partial_varname(
			(parm_type( pl) == tt_str) ? parm_str( pl) : "", &d);
	
	return b ? xstrdup( d->name) : NULL;
}


/** Generates a list of boolean constants.
 */
char *
generator_env_booltype( parm_link_s *pl, const void *data, int level)

{
	static const char **d;

	PRE( pl != NULL);
	PRE( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	// iterator initialization
	if (level == 0)
		d = t_bool;
	else
		d++;

	// find next suitable entry
	while (*d)
	{
		if (prefix( parm_str( pl), *d))
			break;
		d++;
	}

	return *d ? xstrdup( *d) : NULL;
}


/** Generates a list of boolean environment variables.
 */
char *
generator_bool_envname( parm_link_s *pl, const void *data, int level)

{
	bool b;
	static const set_s *d;

	PRE( pl != NULL);
	PRE( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	// iterator initialization
	if (level == 0)
		d = NULL;
	
	// find next suitable entry
	do {
		b = env_by_partial_varname(
				(parm_type( pl) == tt_str) ? parm_str( pl) : "", &d);
	} while (b && d->type != vt_bool);
	
	return b ? xstrdup( d->name) : NULL;
}


/** Generates an equal mark.
 */
char *
generator_equal_char( parm_link_s *pl, const void *data, int level)

{
	PRE( pl != NULL);
	PRE( parm_type( pl) == tt_str || parm_type( pl) == tt_end);
	
	return (level == 0) ? xstrdup( "=") : NULL;
}
