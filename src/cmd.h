/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Reading and executing commands
 *
 */

#ifndef CMD_H_
#define CMD_H_

#include "mtypes.h"
#include "parser.h"

enum {
	CONF_SEC_UNK = 0,
	CONF_SEC_CPU,
	CONF_SEC_DBG,
	CONF_SEC_MEM,
	CONF_SEC_DEV,
	CONF_SEC_END
};

extern bool interpret(const char *str);
extern void script(void);
extern void find_system_generator(parm_link_s **pl, gen_f *generator,
	const void **data);

#endif /* CMD_H_ */
