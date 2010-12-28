/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../signal.h"

#ifdef __WIN32__

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include "../../fault.h"
#include "../../main.h"

static BOOL machine_user_break(DWORD fdwCtrlType)
{
	switch (fdwCtrlType) {
	case CTRL_C_EVENT:
		if ((machine_break) || (machine_interactive)) {
			printf("\n");
			alert("Quit");
			input_back();
			return false;
		}
		
		machine_break = true;
		
		if (!machine_interactive)
			machine_newline = true;
		
		machine_interactive = true;
		
		return true;
	}
	
	return false;
}

void register_sigint(void)
{
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) machine_user_break, true);
}

#endif /* __WIN32__ */
