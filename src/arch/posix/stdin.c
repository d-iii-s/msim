/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../stdin.h"

#ifndef __WIN32__

#include <unistd.h>
#include <sys/select.h>

bool stdin_poll(char *key)
{
	/* Check new character */
	fd_set rfds;
	
	/* Watch stdin */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	int retval = select(1, &rfds, NULL, NULL, &tv);
	
	if (retval == 1) {
		/* There is a new character */
		read(0, key, 1);
		return true;
	}
	
	return false;
}

#endif /* !__WIN32__ */
