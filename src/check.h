/*
 * Pre/post conditions
 * 2004 Viliam Holub
 *
 * Usage: {RQ,PRE,POST}( condition1, condition2, condition3, ...);
 *
 * In the developing stage RQ checks all conditions and displays a warning
 * message about those which does not hold. PRE is used usually at
 * the beginning of functions to check input parameters, where it is doing
 * a very similar job as the "pre-conditions" statements known from some
 * programming languages. RQ may be used almost everywhere, with similar
 * behavior as a "more intelligent" assert.
 * In release stage RQ, PRE and POST are removed by the preprocessor.
 *
 * Feel free to modify this code for your needs.
 *
 * 
 * ISO9899 C99 standard brings the feature of variable number of arguments
 * in macro definitions. Writing more than one pre-condition is than far
 * more comfortable. GCC supports this quite a long time but with different
 * syntax. We try to check the C version (this makes the code less readable).
 */

#ifndef _CHECK_H_
#define _CHECK_H_

/*
 * PARAMETER 1 - Enable checking
 * This may be moved somewhere to the configuration file.
 */
#define RQ_DEBUG

/*
 * PARAMETER 2 - The fatality
 * Enable the RQ_FATAL to break the program on error. The value specifies
 * an exit code.
 */
#define RQ_FATAL 1

/*
 * PARAMETER 3 - Colors
 * Enable the RQ_COLOR to colorize the output.
 */
#define RQ_COLOR


/*
 * Macro definitions
 */
#ifdef RQ_DEBUG
#	define RQ_PARM_BRK -314
#	if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#		define RQ(...)		RQ_test( "", __FILE__, __LINE__,\
			__FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK);
#		define PRE(...)		RQ_test( "pre", __FILE__, __LINE__,\
			__FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK);
#		define POST(...)	RQ_test( "post", __FILE__, __LINE__,\
			__FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK);
#	else
#		define RQ(args...)	RQ_test( "", __FILE__, __LINE__,\
			__FUNCTION__, #args, args, RQ_PARM_BRK);
#		define PRE(args...)	RQ_test( "pre", __FILE__, __LINE__,\
			__FUNCTION__, #args, args, RQ_PARM_BRK);
#		define POST(args...)	RQ_test( "post", __FILE__, __LINE__,\
			__FUNCTION__, #args, args, RQ_PARM_BRK);
#	endif
void RQ_test( const char *pre, const char *filename, int lineno,
		const char *func, const char *term, ...);
#else
#	define RQ( ...)		((void *)0)
#	define PRE( ...)	((void *)0)
#	define POST( ...)	((void *)0)
#endif

#endif /* _CHECK_H_ */
