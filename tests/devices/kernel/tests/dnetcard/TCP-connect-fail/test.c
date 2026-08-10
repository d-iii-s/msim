#include <ktest.h>
#include <drivers/dnetcard.h>
#include <net/ip.h>
#include <net/tcp.h>

#define PROTOCOL_STCP 150
#define TCP_HEADER_SIZE 0x5

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
    ip_header->ip_p = PROTOCOL_STCP;
    ip_header->ip_len = (ip_header->ip_hl + TCP_HEADER_SIZE) * sizeof(uint32_t);

    struct tcphdr *tcp_header = (struct tcphdr *) (buffer + ip_header->ip_hl * sizeof(uint32_t));
    tcp_header->th_off = TCP_HEADER_SIZE;
    tcp_header->th_dport = 0xDCEA; /* 60124 in network byte order where HTTP server is NOT listening */
    tcp_header->th_sport = 0xDCEA;

    netcard->tx_addr_lo = (volatile uint32_t) buffer;
    netcard->status_command = COMMAND_SEND;

    ktest_assert((netcard->status_command & STATUS_INT_TX) == 0, "Netcard TX interrupt should not be pending.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending.");

    while ((netcard->status_command & STATUS_INT_TX) == 0)
        ;

    ktest_assert((netcard->status_command & STATUS_INT_TX) != 0, "Netcard TX interrupt should be pending, detecting TCP connection failure.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) != 0, "Netcard ERR interrupt should be pending, detecting TCP connection failure.");

    ktest_passed();
}
