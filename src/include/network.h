/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#ifdef MINGW

#else

	#include <sys/socket.h> 
	#include <netinet/in.h>

#endif /* MINGW */

#endif /* NETWORK_H_ */
