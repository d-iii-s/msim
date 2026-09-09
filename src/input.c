/*
 * Copyright (c) 2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Input routines
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "../config.h"
#include "arch/console.h"
#include "assert.h"
#include "cmd.h"
#include "fault.h"
#include "input.h"
#include "main.h"
#include "parser.h"
#include "utils.h"

#define PROMPT ("[" PACKAGE_NAME "] ")

/** Shared variables for tab completion */
static char *par_text;
static token_t *last_pars = NULL;
static const void *data;
static gen_t gen;

/** Is the standard input a terminal? */
static bool input_term;

static struct termios tio_shadow;
static struct termios tio_inter;
static struct termios tio_old;

/** Generate all possible words suitable for completion
 *
 */
static char *hint_generator(const char *input, int level)
{
    ASSERT(level >= 0);

    if (level == 0) {
        if (last_pars != NULL) {
            parm_delete(last_pars);
        }

        /* Find completion generator at first. */
        last_pars = parm_parse(par_text);
        parm_check_end(last_pars, par_text);

        data = NULL;
        gen = find_completion_generator(&last_pars, &data);
        if (gen == NULL) {
            return NULL;
        }
    }

    ASSERT(gen != NULL);
    return gen(last_pars, data, level);
}

/** Try to complete the user input
 *
 */
static char **msim_completion(const char *text, int start, int end)
{
    ASSERT(end >= 0);

    char **result;

    par_text = (char *) safe_malloc(end + 1);
    strncpy(par_text, rl_line_buffer, end);
    par_text[end] = 0;

    rl_attempted_completion_over = 1;

    result = rl_completion_matches("", hint_generator);
    safe_free(par_text);

    return result;
}

/** Terminal and readline initialization
 *
 */
void input_init(void)
{
    using_history();
    input_term = !!isatty(0);

    if (!input_term) {
        return;
    }

    (void) tcgetattr(0, &tio_shadow);

    tio_old = tio_shadow;
    tio_inter = tio_shadow;
    tio_shadow.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
    tio_shadow.c_lflag &= ~(IUCLC);
#endif
    tio_shadow.c_cc[VMIN] = 1;
    tio_shadow.c_cc[VTIME] = 0;

    tio_inter.c_lflag &= ~(ECHO | INLCR | ICANON | ECHOE | ONLCR);
#ifdef IUCLC
    tio_inter.c_lflag &= ~(IUCLC);
#endif
    tio_inter.c_cc[VMIN] = 1;
    tio_inter.c_cc[VTIME] = 0;

    rl_readline_name = "msim";
    rl_attempted_completion_function = msim_completion;
}

void input_shadow(void)
{
    if (input_term) {
        (void) tcsetattr(0, TCSANOW, &tio_shadow);
    }
}

void input_back(void)
{
    if (input_term) {
        (void) tcsetattr(0, TCSANOW, &tio_old);
    }
}

/** Interactive mode control
 *
 */
void interactive_control(void)
{
    machine_break = false;

    if (machine_newline) {
        printf("\n");
        machine_newline = false;
    }

    stepping = 0;

    while (machine_interactive) {
        input_back();

        /*
         * Printing the prompt depends on whether the input is a terminal.
         *
         * When the input is a terminal, we let readline() print the prompt
         * itself. This keeps the prompt on screen after a screen refresh
         * (CTRL+L) -- readline needs to own the prompt string in order to be
         * able to redraw it. Passing NULL here (or setting rl_already_prompted)
         * would leave the top of the screen without the prompt after such a
         * refresh.
         *
         * When the input is not a terminal (e.g. a file with a list of
         * commands, or /dev/null), we print the prompt manually and pass NULL
         * to readline(). This is done for consistency between the two readline
         * implementations: GNU readline writes the prompt to the output even on
         * a non-tty input, whereas the BSD editline (libedit) shipped with
         * macOS does not. Printing it ourselves and giving readline a NULL
         * prompt makes the output identical on both platforms, which matters
         * for the test outputs.
         *
         * Note that rl_already_prompted is not honoured by libedit, so it can
         * not be combined with a non-NULL prompt to achieve the same -- that
         * would print the prompt twice on macOS.
         *
         * The two implementations still differ in whether commands read from a
         * non-tty input are echoed to the output (GNU echoes them, libedit does
         * not), so tests that feed a command file into the interactive mode are
         * not portable yet.
         */
        if (!input_term) {
            printf("%s", PROMPT);
            fflush(stdout);
            rl_already_prompted = 1;
        }

        char *cmdline = readline(input_term ? PROMPT : NULL);

        input_shadow();

        if (!cmdline) {
            /* User break in readline */
            printf("\n");
            alert("Quit");
            input_back();
            free(cmdline);
            exit(ERR_OK);
        }

        if (*cmdline) {
            add_history(cmdline);
            interpret(cmdline);
        } else {
            interpret("step");
        }

        free(cmdline);
    }
}

/**
 * Tells whether stdin is connected to interactive terminal.
 */
int input_is_terminal(void)
{
    return input_term;
}

void input_end(void)
{
    clear_history();
}
