#ifndef _MSIM_H_GUARD
#define _MSIM_H_GUARD

static __attribute__((noreturn)) inline void msim_halt(void)
{
#ifdef ARCH_MIPS32
    __asm__ volatile(".word 0x28\n");
#endif
#ifdef ARCH_RISCV32
    __asm__ volatile(".word 0x8C000073\n");
#endif
    while (1) {
    }
}

#endif
