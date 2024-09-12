// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 Charles University

/*
 * Aliases for easier reading.
 */
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;


/** Display a single character.
 *
 * @param c The character to display.
 */
static inline void print_char(const char c) {
    // Note that this is a virtual address
    // (compare with settings in msim.conf).
    volatile char *printer = (volatile char*)(0x90000000);
    *printer = c;
}

static void dump_uint32(uint32_t value) {
    while (value > 0) {
        print_char("0123456789abcdef"[value % 16]);
        value /= 16;
    }
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

/*
 * This variable is volatile to ensure compiler does not optimize
 * the whole code away.
 */
static volatile int offset = 1;

void kernel_main(void) {
    uint8_t array[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    
    uint32_t *val32 = (uint32_t *)array;
    dump_uint32(val32[0]);
    
    val32 = (uint32_t *)(array + offset);
    dump_uint32(val32[0]);
}
