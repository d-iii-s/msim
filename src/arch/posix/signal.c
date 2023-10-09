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
#include "../../input.h"
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

static void termination_signals_handler(int signo)
{
    printf("\n");
    alert("Caught signal %d, terminating.", signo);
    input_back();
    exit(ERR_OK);
}

void register_signal_handlers(void)
{
    struct sigaction act;

    act.sa_handler = machine_user_break;
    (void) sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    struct sigaction act_term;
    act_term.sa_handler = termination_signals_handler;
    (void) sigemptyset(&act_term.sa_mask);
    act_term.sa_flags = 0;
    sigaction(SIGTERM, &act_term, NULL);
    sigaction(SIGQUIT, &act_term, NULL);
}

#endif /* !__WIN32__ */
