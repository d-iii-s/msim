#include <ktest.h>
#include <drivers/dnetcard.h>
#include <net/ip.h>
#include <net/udp.h>

#define PROTOCOL_UDP 17

void kernel_test(void)
{
    ktest_start("dnetcard/interrupt");

    volatile netcard_t *netcard = (netcard_t *) NETCARD_ADDRESS;
    netcard->ip_address = 0x0100000A; /* 10.0.0.1 in network byte order */

    char buffer[1500];
    struct ip *ip_header = (struct ip *) buffer;
    ip_header->ip_hl = 0x5;
    ip_header->ip_dst.s_addr = 0x0100007F;
    ip_header->ip_src.s_addr = netcard->ip_address;
    ip_header->ip_p = PROTOCOL_UDP;
    ip_header->ip_len = ip_header->ip_hl * sizeof(uint32_t) + sizeof(struct udphdr);
    struct udphdr *udp_header = (struct udphdr *) (buffer + ip_header->ip_hl * sizeof(uint32_t));
    udp_header->uh_dport = 0xDBEA; /* 60123 in network byte order */
    udp_header->uh_sport = 0xDBEA;
    udp_header->uh_ulen = sizeof(struct udphdr) + 0;

    netcard->tx_addr_lo = (volatile uint32_t) buffer;
    netcard->status_command = COMMAND_SEND;

    ktest_assert((netcard->status_command & STATUS_INT_TX) == 0, "Netcard TX interrupt should not be pending.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending.");

    // waiting for packet to be sent
    while ((netcard->status_command & STATUS_INT_TX) == 0)
        ;

    ktest_assert((netcard->status_command & STATUS_INT_TX) != 0, "Netcard TX interrupt should be pending after sending a packet.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending after sending UDP packet.");

    ktest_passed();
}
