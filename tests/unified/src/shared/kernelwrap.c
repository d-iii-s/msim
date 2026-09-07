#include <kernel.h>
#include <msim.h>

void kernel_wrap() {
    kernel_main();
    msim_halt();
}

