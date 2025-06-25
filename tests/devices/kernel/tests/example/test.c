#include <ktest.h>

void kernel_test(void) {
    ktest_start("basic/stack_pointer");

    ktest_assert(true, "Should succeed.");

    ktest_passed();
}
