#include <ktest.h>
#include <drivers/dnetcard.h>

void kernel_test(void)
{
    ktest_start("dnetcard/read-write-registers");

    volatile netcard_t *netcard = (netcard_t *) NETCARD_ADDRESS;

    netcard_t previous;
    previous.tx_addr_lo = netcard->tx_addr_lo;
    previous.tx_addr_hi = netcard->tx_addr_hi;
    previous.rx_addr_lo = netcard->rx_addr_lo;
    previous.rx_addr_hi = netcard->rx_addr_hi;
    previous.ip_address = netcard->ip_address;

    netcard->tx_addr_lo++;
    netcard->tx_addr_hi++;
    netcard->rx_addr_lo++;
    netcard->rx_addr_hi++;
    netcard->ip_address++;

    ktest_assert(netcard->tx_addr_lo != previous.tx_addr_lo, "Value of TX_ADDR_LO register should be %x but was %x.", previous.tx_addr_lo + 1, netcard->tx_addr_lo);
    ktest_assert(netcard->tx_addr_hi != previous.tx_addr_hi, "Value of TX_ADDR_HI register should be %x but was %x.", previous.tx_addr_hi + 1, netcard->tx_addr_hi);
    ktest_assert(netcard->rx_addr_lo != previous.rx_addr_lo, "Value of RX_ADDR_LO register should be %x but was %x.", previous.rx_addr_lo + 1, netcard->rx_addr_lo);
    ktest_assert(netcard->rx_addr_hi != previous.rx_addr_hi, "Value of RX_ADDR_HI register should be %x but was %x.", previous.rx_addr_hi + 1, netcard->rx_addr_hi);
    ktest_assert(netcard->ip_address != previous.ip_address, "Value of IP_ADDRESS register should be %x but was %x.", previous.ip_address + 1, netcard->ip_address);

    ktest_passed();
}
