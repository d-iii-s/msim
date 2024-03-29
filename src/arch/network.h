/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#ifdef __WIN32__

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <netinet/in.h>
#include <sys/socket.h>

#endif /* __WIN32__ */

#endif
