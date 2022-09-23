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

#ifndef INPUT_H_
#define INPUT_H_

extern void input_init(void);
extern void input_end(void);
extern void input_shadow(void);
extern void input_back(void);
extern void interactive_control(void);
extern int input_is_terminal(void);

#endif
