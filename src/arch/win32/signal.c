/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../signal.h"

#ifdef __WIN32__

#include <windows.h>
#include <stdlib.h>
#include "../../device/machine.h"
#include "../../fault.h"

static BOOL machine_user_break(DWORD fdwCtrlType)
{
	switch (fdwCtrlType) {
	case CTRL_C_EVENT:
		if ((tobreak) || (interactive)) {
			alert("Quit");
			input_back();
			return false;
		}
		
		tobreak = true;
		if (!interactive)
			reenter = true;
		interactive = true;
		return true;
	}
	
	return false;
}

void register_sigint(void)
{
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) machine_user_break, true);
}

#endif /* __WIN32__ */
