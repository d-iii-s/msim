/*
 * Copyright (c) 2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Remote debugging
 *
 */

#ifndef GDB_H_
#define GDB_H_

#include <stdbool.h>

#include "../mtypes.h"

extern bool gdb_remote_init(void);
extern void gdb_session(int event);
extern void gdb_handle_event(int event);
	
#endif /* GDB_H_ */
