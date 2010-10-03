/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../signal.h"

#ifndef __WIN32__

#include <signal.h>
#include <stdlib.h>
#include "../../device/machine.h"
#include "../../io/output.h"

static void machine_user_break(int signo)
{
	if ((tobreak) || (interactive)) {
		mprintf("Quit\n");
		input_back();
		exit(1);
	}
	
	tobreak = true;
	if (!interactive)
		reenter = true;
	interactive = true;
}

void register_sigint(void)
{
	struct sigaction act;
	
	act.sa_handler = machine_user_break;
	(void) sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);
}

#endif /* !__WIN32__ */
