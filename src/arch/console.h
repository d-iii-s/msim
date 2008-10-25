/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#ifdef __WIN32__

/**
 *
 * Dummy definitions for compatibility
 * with termios.h.
 *
 */

#define tcgetattr(fd, termios)  0
#define tcsetattr(fd, optional_actions, termios)  0

#define VTIME    5
#define VMIN     6

#define TCSANOW  0

#define INLCR    0000100
#define ICANON   0000002

#define ECHOE    0000020
#define ECHO     0000010

#define ONLCR    0000004

#define NCCS     32

typedef unsigned char cc_t;
typedef unsigned int tcflag_t;

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
};


#else

#include <termios.h>

#endif /* __WIN32__ */

#endif /* CONSOLE_H_ */
