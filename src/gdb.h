/*
 * gdb.h
 * Remote GDB
 * Copyright (c) 2003 Viliam Holub
 */


#ifndef _GDB_H_
#define _GDB_H_

#include "mtypes.h"

bool gdb_remote_init( void);
void gdb_session( int event);
void gdb_handle_event( int event);
	
#endif /* _GDB_H_ */
