#ifndef _KERNEL_H_GUARD
#define _KERNEL_H_GUARD

extern void kernel_wrap(void);
extern void kernel_main(void);

static inline void kputs(const char *s)
{
    volatile char *printer = (volatile char *) 0x90000000;
    while (*s != 0) {
        *printer = *s;
        s++;
    }
    *printer = '\n';
}

#endif
