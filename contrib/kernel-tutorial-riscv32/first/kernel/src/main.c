// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 Charles University

/** Display a single character.
 *
 * @param c The character to display.
 */
static inline void print_char(const char c)
{
    // Note that this is a virtual address
    // (compare with settings in msim.conf).
    volatile char *printer = (volatile char *) (0x90000000);
    *printer = c;
}

/** Halt the execution of the simulator */
static inline void halt(void)
{
    __asm__ volatile(".word 0x8C000073");
}

/** This is kernel C-entry point.
 *
 * Kernel jumps here from assembly bootstrap code. Note that
 * this function runs on special stack and does not represent a
 * real thread.
 *
 * Note that normally the prototype (first line) is defined in
 * a header file. Since we call this function only from assembler,
 * it is not really needed.
 */
void kernel_main(void);

void kernel_main(void)
{
    print_char('H');
    print_char('e');
    print_char('l');
    print_char('l');
    print_char('o');
    print_char(',');
    print_char(' ');
    print_char('W');
    print_char('o');
    print_char('r');
    print_char('l');
    print_char('d');
    print_char('.');
    print_char('\n');
    halt();
}
