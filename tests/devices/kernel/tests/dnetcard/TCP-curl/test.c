#include <ktest.h>
#include <drivers/dnetcard.h>
#include <net/ip.h>
#include <net/tcp.h>

#define PROTOCOL_STCP 150
#define TCP_HEADER_SIZE 0x5
#define DATA_SIZE 20

void kernel_test(void)
{
    ktest_start("dnetcard/interrupt");

    volatile netcard_t *netcard = (netcard_t *) NETCARD_ADDRESS;
    netcard->ip_address = 0x0100000A; /* 10.0.0.1 in network byte order */

    char out_buffer[1500];
    struct ip *ip_header = (struct ip *) out_buffer;
    ip_header->ip_hl = 0x5;
    ip_header->ip_dst.s_addr = 0x0100007F;
    ip_header->ip_src.s_addr = netcard->ip_address;
    ip_header->ip_p = PROTOCOL_STCP;
    ip_header->ip_len = (ip_header->ip_hl + TCP_HEADER_SIZE) * sizeof(uint32_t) + DATA_SIZE;

    struct tcphdr *tcp_header = (struct tcphdr *) (out_buffer + ip_header->ip_hl * sizeof(uint32_t));
    tcp_header->th_off = TCP_HEADER_SIZE;
    tcp_header->th_dport = 0xDBEA; /* 60123 in network byte order where HTTP server IS listening */
    tcp_header->th_sport = 0xDBEA;

    char *data_ptr = (char *) (out_buffer + (ip_header->ip_hl + tcp_header->th_off) * sizeof(uint32_t));
    char *request = "GET /file.txt\n\n";
    while (*request) {
        *data_ptr = *request;
        data_ptr++;
        request++;
    }

    volatile char in_buffer[1500] = { 0 };

    netcard->tx_addr_lo = (volatile uint32_t) out_buffer;
    netcard->rx_addr_lo = (volatile uint32_t) in_buffer;
    netcard->status_command = COMMAND_RECEIVE;
    netcard->status_command = COMMAND_SEND;

    ktest_assert((netcard->status_command & STATUS_INT_TX) == 0, "Netcard TX interrupt should not be pending.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending.");

    while ((netcard->status_command & STATUS_INT_TX) == 0)
        ;

    ktest_assert((netcard->status_command & STATUS_INT_TX) != 0, "Netcard TX interrupt should be pending, detecting sent packet.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending, connection should have been succesfull.");

    netcard->status_command = COMMAND_INT_TX_ACK;

    while ((netcard->status_command & STATUS_INT_RX) == 0)
        ;

    ktest_assert((netcard->status_command & STATUS_INT_RX) != 0, "Netcard RX interrupt should be pending, detecting received packet.");
    ktest_assert((netcard->status_command & STATUS_INT_ERR) == 0, "Netcard ERR interrupt should not be pending.");

    ip_header = (struct ip *) in_buffer;
    tcp_header = (struct tcphdr *) (in_buffer + ip_header->ip_hl * sizeof(uint32_t));
    ktest_assert(ip_header->ip_dst.s_addr == 0x0100000A, "Destination address should be 10.0.0.1, but was different.");
    ktest_assert(ip_header->ip_src.s_addr == 0x0100007F, "Source address should be 127.0.0.1, but was different.");
    ktest_assert(tcp_header->th_dport == 0xDBEA, "Destination port should be 60123 (0xDBEA), but was different.");
    ktest_assert(tcp_header->th_sport == 0xDBEA, "Source port should be 60123 (0xDBEA), but was different.");

    ktest_passed();
}
