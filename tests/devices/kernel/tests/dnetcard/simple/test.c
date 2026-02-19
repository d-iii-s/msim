#include <ktest.h>
#include <drivers/dnetcard.h>

void kernel_test(void)
{
    ktest_start("dnetcard/simple");

    volatile netcard_t *netcard = (netcard_t *) NETCARD_ADDRESS;
    ktest_assert((netcard->status_command & STATUS_RECEIVE) == 0, "Netcard should not be receiving before told so.");

    ktest_passed();
}
