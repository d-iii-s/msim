/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../signal.h"

#ifndef __WIN32__

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "../../fault.h"
#include "../../main.h"

static void machine_user_break(int signo)
{
	if ((machine_break) || (machine_interactive)) {
		printf("\n");
		alert("Quit");
		input_back();
		exit(ERR_OK);
	}
	
	machine_break = true;
	
	if (!machine_interactive)
		machine_newline = true;
	
	machine_interactive = true;
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
