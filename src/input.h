/*
 * Input routines
 *
 * Copyright (c) 2004 Viliam Holub
 */

#ifndef _INPUT_H_
#define _INPUT_H_

void input_init( void);
void input_inter( void);
void input_shadow( void);
void input_back( void);
char *hint_generator( const char *input, int level);
char **msim_completion( const char *text, int start, int end);
void interactive_control( void);

#endif // _INPUT_H_
