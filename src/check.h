/*
 * Copyright (c) 2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Pre/post conditions
 *
 * Usage: {RQ,PRE,POST}(condition1, condition2, condition3, ...);
 *
 * In the developing stage RQ checks all conditions and displays a warning
 * message about those which does not hold. PRE is used usually at
 * the beginning of functions to check input parameters, where it is doing
 * a very similar job as the "pre-conditions" statements known from some
 * programming languages. RQ may be used almost everywhere, with similar
 * behavior as a "more intelligent" assert.
 *
 * In release stage RQ, PRE and POST are removed by the preprocessor.
 * 
 */

#ifndef CHECK_H_
#define CHECK_H_

/** Enable checking
 *
 * Enable checking of pre/post conditions.
 *
 */
#define RQ_DEBUG

/** Eable fatality
 *
 * Enable to break the program on error.
 * The value specifies an exit code.
 *
 */
#define RQ_FATAL 1

/** Enable color output
 *
 * Enable to colorize the output.
 *
 */
#define RQ_COLOR


#ifdef RQ_DEBUG
	#define RQ_PARM_BRK -314
	#define RQ(...)   rq_test("", __FILE__, __LINE__, __FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK)
	#define PRE(...)  rq_test("pre", __FILE__, __LINE__, __FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK)
	#define POST(...) rq_test("post", __FILE__, __LINE__, __FUNCTION__, #__VA_ARGS__, __VA_ARGS__, RQ_PARM_BRK)
#else
	#define RQ(...)		
	#define PRE(...)	
	#define POST(...)	
#endif


extern void rq_test(const char *pre, const char *filename, int lineno,
	const char *func, const char *term, ...);


#endif /* CHECK_H_ */
