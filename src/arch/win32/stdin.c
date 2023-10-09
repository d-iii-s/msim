/*
 * Copyright (c) 2008 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 */

#include "../stdin.h"

#ifdef __WIN32__

#include <windows.h>

bool stdin_poll(char *key)
{
    HANDLE stdin = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD inrec;
    DWORD rd;

    do {
        if (PeekConsoleInput(stdin, &inrec, 1, &rd)) {
            if (rd > 0) {
                if (!ReadConsoleInput(stdin, &inrec, 1, &rd))
                    return false;

                if ((rd > 0) && (inrec.EventType == KEY_EVENT)
                    && (inrec.Event.KeyEvent.bKeyDown)) {
                    *key = inrec.Event.KeyEvent.uChar.AsciiChar;
                    return true;
                }
            }
        } else {
            if (!PeekNamedPipe(stdin, NULL, 0, NULL, &rd, NULL))
                return false;

            if (rd > 0) {
                if (!ReadFile(stdin, key, 1, &rd, NULL))
                    return false;

                if (rd > 0)
                    return true;
            }
        }
    } while (rd > 0);

    return false;
}

#endif /* __WIN32__ */
