/*
 * Verbose assert
 * 2004 Viliam Holub
 *
 * Usage: RQ( condition1, condition2, condition3, ...);
 *
 * In the developing stage RQ checks all conditions and diplays a warning
 * message about these which does not hold. RQ is used usually at
 * the beginnig of functions to check input parameters, where it is doing
 * a very similar job as the "pre-conditions" statements in some programming
 * languages. But may be used almost everywhere, with similar behavior as
 * a "more intelligent" assert.
 * In release stage RQ is removed by the preprocessor.
 *
 * Feel free to modify this code for your needs.
 *
 * 
 * ISO9899 C99 standard brings the feature of variable number of arguments
 * in macro definitions. Writting more than one pre-condition is than far
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
 * the exit code.
 */
#define RQ_FATAL -1


/*
 * Macro definitions
 */
#ifdef RQ_DEBUG
#	define RQ_PARM_BRK -314
#	if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#		define RQ(...)		RQ_test( __FILE__, __LINE__,\
			__FUNCTION__, __VA_ARGS__, RQ_PARM_BRK, #__VA_ARGS__);
#	else
#		define RQ(args...)	RQ_test( __FILE__, __LINE__,\
			__FUNCTION__, args, RQ_PARM_BRK, #args);
#endif
void RQ_test( const char *filename, int lineno, const char *func, ...);
#else
#	define RQ( ...)
#endif

#endif /* _CHECK_H_ */
