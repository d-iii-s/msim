// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Charles University

#define PAGETABLE_PHYS 0xA0000000

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

static inline void ebreak(void)
{
    __asm__ volatile("ebreak");
}

/** Halt the execution of the simulator */
static inline void halt(void)
{
    __asm__ volatile(".word 0x8C000073");
}

static inline void set_pagetable(unsigned pagetable_physical_address) {
    unsigned satp_value = 0x80000000 | (pagetable_physical_address >> 12);
    __asm__ volatile("csrw satp, %0\n"::"r"(satp_value));
}

static inline char read_from_address(unsigned address) {
    volatile char* ptr = (volatile char*)address;
    return *ptr;
}

static inline void write_to_address(unsigned address, char value) {
    volatile char* ptr = (volatile char*)address;
    *ptr = value;
}

static void greet(void) {
    print_char('V');
    print_char('i');
    print_char('r');
    print_char('t');
    print_char('u');
    print_char('a');
    print_char('l');
    print_char(' ');
    print_char('h');
    print_char('i');
    print_char('!');
    print_char('\n');
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
    set_pagetable(PAGETABLE_PHYS);
    ebreak();

    greet();
    ebreak();

    
    write_to_address(0xB0000000, 'A');
    char value0 = read_from_address(0xB0000000);
    print_char(value0);

    // char value1 = read_from_address(0xC0001000); // Invalid translation

    // write_to_address(0xC0002000, 'B'); // Read-only page

    char value2 = read_from_address(0xB0002000);
    print_char(value2);

    print_char('\n');

    halt();
}
