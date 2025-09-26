#pragma once

#include <types.h>

#define true 1
#define false 0
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void* memset(void* buf, char c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
char* strcpy(char* dst, const char* src);
int strcmp(const char* s1, const char* s2);
void putchar(char ch);
void printk(const char* fmt, ...);
void puts(const char* str);
