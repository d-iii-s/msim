#include <kernel.h>

volatile int blackhole = 0;

void kernel_main()
{
    volatile int *dnomem = (volatile int *) 0x88000008;
    kputs("Will read from dnomem area");
    blackhole = dnomem[0];
    kputs("Will write into dnomem area");
    dnomem[0] = 42;
}
