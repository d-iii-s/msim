/*
 * Asserts-like functions
 * 2004 Viliam Holub
 *
 * ISO C99 standard brings a feature of variable number of arguments
 * in macro definition. The syntax is similar and we use it. Writting
 * of more than one pre-conditions is far more comfortable than splitting
 * them. GCC supports this quite a long time but with different syntax.
 * I try use both syntax clearly
 * 
 */

#ifndef _CHECK_H_
#define _CHECK_H_

/*
 * Enable checking
 * This may be moved to the configuration.
 */
#define RQ_DEBUG

/*
 * Enable the fatality
 * Enbale the RQ_FATAL to break the program on error. The value specifies
 * the exit code.
 */
#define RQ_FATAL -1

/*
 * Macro definitions
 */
#ifdef RQ_DEBUG
#	define RQ_PARM_BRK -314
#	define RQ(...)	RQ_test( __FILE__, __LINE__, __FUNCTION__,\
		__VA_ARGS__, RQ_PARM_BRK, #__VA_ARGS__);
void RQ_test( const char *filename, int lineno, const char *func, ...);
#else
#	define RQ( ...)
#endif

#endif /* _CHECK_H_ */
