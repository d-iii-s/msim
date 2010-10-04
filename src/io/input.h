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
extern void input_inter(void);
extern void input_shadow(void);
extern void input_back(void);
extern char *hint_generator(const char *input, int level);
extern char **msim_completion(const char *text, int start, int end);
extern void interactive_control(void);

#endif
