/*
 * reading and executing configuration
 * Copyright (c) 2002-2004 Viliam Holub
 */

#ifndef _CONF_H_
#define _CONF_H_

#include "mtypes.h"
#include "parser.h"

enum
{
	CONF_SEC_UNK=0, CONF_SEC_CPU, CONF_SEC_DBG, CONF_SEC_MEM,
	CONF_SEC_DEV, CONF_SEC_END
};


bool interpret( const char *s, int lineno);
void script();
void find_system_generator( parm_link_s **pl, gen_f *generator,
		const void **data);

#endif /* _CONF_H_ */
