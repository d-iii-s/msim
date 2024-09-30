// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 Charles University

volatile long *clock = (volatile long *) (0x90000200);
volatile char *printer = (volatile char *) (0x90000000);
volatile long counter = 0;

static inline void print_char(const char c)
{
    *printer = c;
}

static inline void halt(void)
{
    __asm__ volatile(".word 0x8C000073");
}

static void print_unsigned(unsigned long value)
{
    volatile char *alphabet = "0123456789";
    if (value > 9) {
        print_unsigned(value / 10);
    }
    print_char(alphabet[value % 10]);
}

static void message(const char *msg, unsigned long value)
{
    volatile char *it = (volatile char *) msg;
    while (*it != 0) {
        print_char(*it);
        it++;
    }
    print_unsigned(value);
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
    volatile long counter_start = 0;
    volatile long counter_end;
    volatile long previous_clock = *clock;
    while (1) {
        volatile long clock_now = *clock;
        if (clock_now != previous_clock) {
            message("Clock diff: ", clock_now);
            if (counter_start > 0) {
                counter_end = counter;
                break;
            }
            counter_start = counter;
            previous_clock = clock_now;
        }
        counter++;
    }

    message("Cycles per second: ", counter_end - counter_start);
    halt();
}
