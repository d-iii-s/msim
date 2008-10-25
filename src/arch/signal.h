/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#ifdef __WIN32__

/**
 *
 * Dummy definitions for compatibility
 * with signal.h.
 *
 */

#define sigemptyset(set)  0
#define sigaction(signum, act, oldact)  0

#define _SIGSET_NWORDS  (1024 / (8 * sizeof(unsigned long int)))

typedef struct {
	unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;

typedef void (*__sighandler_t)(int);

struct sigaction {
	__sighandler_t sa_handler;
	__sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};


#else

#include <signal.h>

#endif /* __WIN32__ */

#endif /* SIGNAL_H_ */
