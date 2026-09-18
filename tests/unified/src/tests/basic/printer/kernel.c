#include <kernel.h>

#define PRINTER_ADDRESS 0x90000000

static inline void putc(char symbol)
{
    volatile char *device = (volatile char *) PRINTER_ADDRESS;
    *device = symbol;
}

void kernel_main()
{
    putc('H');
    putc('e');
    putc('l');
    putc('l');
    putc('o');
    putc('!');
    putc('\n');
}
