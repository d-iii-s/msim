/*
 * Copyright (c) 2025 Petr Hrdina
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Network card device
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "../fault.h"
#include "../physmem.h"
#include "../text.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "dnetcard.h"

/** Actions the network card is performing */
enum action_e {
    ACTION_NONE, /**< Network card is idle */
    ACTION_PROCESS, /**< Network card is sending/receiving a packet */
};

/* Register offsets */
#define REGISTER_TX_ADDR_LO 0 /* Address (bits 0 .. 31) */
#define REGISTER_TX_ADDR_HI 4 /* Address (bits 32 .. 35) */
#define REGISTER_RX_ADDR_LO 8 /* Address (bits 0 .. 31) */
#define REGISTER_RX_ADDR_HI 12 /* Address (bits 32 .. 35) */
#define REGISTER_STATUS 16 /* Status/commands */
#define REGISTER_COMMAND 16 /* Status/commands */
#define REGISTER_LIMIT 24 /* Size of register block */

/* Status flags */
#define STATUS_RECEIVE 0x02 /* Ready for receiving */
#define STATUS_INT_TX 0x04 /* Tx interrupt pending */
#define STATUS_INT_RX 0x08 /* Rx interrupt pending */
#define STATUS_INT_ERR 0x10 /* Error interrupt pending */
#define STATUS_MASK 0x1f /* Status mask */

/* Command flags */
#define COMMAND_SEND 0x01 /**< Send packet */
#define COMMAND_RECEIVE 0x02 /**< Set receiving on/off */
#define COMMAND_INT_TX_ACK 0x04 /**< Tx interrupt acknowledge */
#define COMMAND_INT_RX_ACK 0x08 /**< Rx interrupt acknowledge */
#define COMMAND_INT_ERR_ACK 0x10 /* Error interrupt acknowledge */
#define COMMAND_MASK 0x1f /* Command mask */

/* Constants */
#define IP_PACKET_SIZE 1500 // typical MTU
#define IP_HEADER_LEN 0x5
#define TCP_HEADER_OFF 0x5
#define TX_BUFFER_SIZE IP_PACKET_SIZE
#define RX_BUFFER_SIZE IP_PACKET_SIZE

typedef struct {
    item_t item;
    struct sockaddr_in dest_addr;
    struct sockaddr_in src_addr;
    int socket;
} connection_t;

/** Network card instance data structure */
typedef struct {
    uint32_t *txbuffer; /* Transfer buffer */
    uint32_t *rxbuffer; /* Receive buffer */
    list_t opened_conns;
    size_t opened_count;
    list_t listening_conns;
    size_t listening_count;

    /* Configuration */
    ptr36_t addr; /* Register address */
    unsigned int intno; /* Interrupt number */

    /* Registers */
    ptr36_t tx_ptr; /* Current tx DMA pointer */
    ptr36_t rx_ptr; /* Current rx DMA pointer */
    uint32_t status; /* Network card status register */
    uint32_t command; /* Network card command register */

    /* Current action variables */
    enum action_e tx_action; /* Action type */
    enum action_e rx_action; /* Action type */
    size_t tx_cnt; /* Word counter */
    size_t rx_cnt; /* Word counter */
    bool ig; /* Interrupt pending flag */

    /* Statistics */
    uint64_t intrcount; /* Number of interrupts */
    uint64_t packets_tx; /* Number of transferred packets */
    uint64_t packets_rx; /* Number of received packets */
    uint64_t cmds_error; /* Number of illegal commands */

} netcard_data_s;

/** Create new outgoing connection
 *
 * @param netcard Network card instance data structure
 * @param addr Connection address structure
 *
 * @return Socket file descriptor, -1 on errors
 */
static int dnetcard_create_outgoing_connection(netcard_data_s *netcard, struct sockaddr_in *dest_addr, struct sockaddr_in *src_addr)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        // todo add error only in verbose mode
        // improve error message, add addr and port
        io_error("Failed to create socket");
        return -1;
    }

    if (connect(fd, (struct sockaddr *) dest_addr, sizeof(struct sockaddr_in)) == -1) {
        io_error("Failed to connect to address");
        close(fd);
        return -1;
    }

    connection_t *new_conn = malloc(sizeof(connection_t));
    new_conn->dest_addr.sin_family = dest_addr->sin_family;
    new_conn->dest_addr.sin_addr = dest_addr->sin_addr;
    new_conn->dest_addr.sin_port = dest_addr->sin_port;
    new_conn->src_addr.sin_family = src_addr->sin_family;
    new_conn->src_addr.sin_addr = src_addr->sin_addr;
    new_conn->src_addr.sin_port = src_addr->sin_port;
    new_conn->socket = fd;
    item_init(&new_conn->item);
    list_append(&netcard->opened_conns, &new_conn->item);
    netcard->opened_count++;

    return fd;
}

/** Create new incoming connection
 *
 * @param netcard Network card instance data structure
 * @param listen_conn Listening connection from which to accept new connection
 *
 * @return Socket file descriptor, -1 on errors
 */
static int dnetcard_create_incoming_connection(netcard_data_s *netcard, connection_t *listen_conn)
{
    connection_t *new_conn = malloc(sizeof(connection_t));
    socklen_t len = sizeof(new_conn->dest_addr);
    int newsock = accept(listen_conn->socket, (struct sockaddr *) &new_conn->dest_addr, &len);
    if (newsock == -1) {
        io_error("Error accepting a connection");
        free(new_conn);
        return -1;
    }

    new_conn->socket = newsock;
    new_conn->src_addr.sin_family = listen_conn->src_addr.sin_family;
    new_conn->src_addr.sin_addr.s_addr = listen_conn->src_addr.sin_addr.s_addr;
    new_conn->src_addr.sin_port = listen_conn->src_addr.sin_port;

    item_init(&new_conn->item);
    list_append(&netcard->opened_conns, &new_conn->item);
    netcard->opened_count++;
    return newsock;
}

/** Get an open connection socket
 *
 * @param netcard Network card instance data structure
 * @param addr Connection address structure
 *
 * @return File descriptor of the socket, -1 if connection does not exist
 */
static int dnetcard_get_connection(netcard_data_s *netcard, struct sockaddr_in *dest_addr, struct sockaddr_in *src_addr)
{
    connection_t *conn;
    for_each(netcard->opened_conns, conn, connection_t)
    {
        if (conn->dest_addr.sin_addr.s_addr == dest_addr->sin_addr.s_addr && conn->dest_addr.sin_port == dest_addr->sin_port
                && conn->src_addr.sin_addr.s_addr == src_addr->sin_addr.s_addr && conn->src_addr.sin_port == src_addr->sin_port) {
            return conn->socket;
        }
    }

    return -1;
}

/** Close opened connection by fd
 *
 * @param netcard Network card instance data structure
 * @param fd File descriptor number of the connection
 */
static void dnetcard_close_connection(netcard_data_s *netcard, int fd)
{
    connection_t *conn;
    for_each(netcard->opened_conns, conn, connection_t)
    {
        if (conn->socket == fd) {
            close(fd);
            list_remove(&netcard->opened_conns, &conn->item);
            free(conn);
            netcard->opened_count--;
            return;
        }
    }
}

/** Send packet stored in txbuffer
 *
 * @param netcard Network card instance data structure
 *
 * @return true on success, false on error
 */
static bool dnetcard_send_packet(netcard_data_s *netcard)
{
    struct ip *ip_header = (struct ip *) netcard->txbuffer;
    struct tcphdr *tcp_header = (struct tcphdr *) (netcard->txbuffer + ip_header->ip_hl);

    struct sockaddr_in dest_in = { 0 };
    dest_in.sin_family = AF_INET;
    dest_in.sin_port = tcp_header->th_dport;
    dest_in.sin_addr = ip_header->ip_dst;

    struct sockaddr_in src_in = { 0 };
    src_in.sin_family = AF_INET;
    src_in.sin_port = tcp_header->th_sport;
    src_in.sin_addr = ip_header->ip_src;

    int fd;

    if ((fd = dnetcard_get_connection(netcard, &dest_in, &src_in)) == -1) {
        if ((fd = dnetcard_create_outgoing_connection(netcard, &dest_in, &src_in)) == -1) {
            return false;
        }
    }

    char *packet_data = (char *) tcp_header + tcp_header->th_off * sizeof(uint32_t);
    size_t data_size = ip_header->ip_len - (ip_header->ip_hl + tcp_header->th_off) * sizeof(uint32_t);

    write(fd, packet_data, data_size);

    return true;
}

/** Read data for DMA transfer
 *
 * @param netcard Network card instance data structure
 * @param conn Ready connection to read data from
 *
 * @return true on success, false on error
 */
static bool dnetcard_receive_packet(netcard_data_s *netcard, connection_t *conn)
{
    struct ip *ip_header = (struct ip *) netcard->rxbuffer;
    ip_header->ip_hl = IP_HEADER_LEN;
    struct in_addr src_addr = { conn->dest_addr.sin_addr.s_addr };
    struct in_addr dest_addr = { conn->src_addr.sin_addr.s_addr };
    ip_header->ip_src = src_addr;
    ip_header->ip_dst = dest_addr;

    struct tcphdr *tcp_header = (struct tcphdr *) (netcard->rxbuffer + ip_header->ip_hl);
    tcp_header->th_off = TCP_HEADER_OFF;
    tcp_header->th_sport = conn->dest_addr.sin_port;
    tcp_header->th_dport = conn->src_addr.sin_port;

    char *data_ptr = (char *) tcp_header + tcp_header->th_off * sizeof(uint32_t);

    ssize_t data_size = read(conn->socket, data_ptr, IP_PACKET_SIZE - (ip_header->ip_hl + tcp_header->th_off) * sizeof(uint32_t));
    if (data_size == 0 || data_size == -1) {
        return false;
    }

    ip_header->ip_len = (ip_header->ip_hl + tcp_header->th_off) * sizeof(uint32_t) + data_size;
    return true;
}

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dnetcard_init(token_t *parm, device_t *dev)
{
    parm_next(&parm);
    uint64_t _addr = parm_uint_next(&parm);
    uint64_t _intno = parm_uint_next(&parm);

    if (!phys_range(_addr)) {
        error("Physical memory address out of range");
        return false;
    }

    if (!phys_range(_addr + (uint64_t) REGISTER_LIMIT)) {
        error("Invalid address, registers would exceed the physical "
              "memory range");
        return false;
    }

    ptr36_t addr = _addr;

    if (!ptr36_dword_aligned(addr)) {
        error("Physical memory address must be 8-byte aligned");
        return false;
    }

    if (_intno > MAX_INTRS) {
        error("%s", txt_intnum_range);
        return false;
    }

    /* Allocate structure */
    netcard_data_s *netcard = safe_malloc_t(netcard_data_s);
    dev->data = netcard;

    /* Initialization */
    list_init(&netcard->opened_conns);
    netcard->addr = addr;
    netcard->intno = _intno;
    netcard->txbuffer = safe_malloc(TX_BUFFER_SIZE);
    netcard->rxbuffer = safe_malloc(RX_BUFFER_SIZE);
    netcard->tx_ptr = 0;
    netcard->status = 0;
    netcard->command = 0;
    netcard->tx_action = ACTION_NONE;
    netcard->rx_action = ACTION_NONE;
    netcard->tx_cnt = 0;
    netcard->rx_cnt = 0;
    netcard->ig = false;
    netcard->intrcount = 0;
    netcard->packets_tx = 0;
    netcard->packets_rx = 0;
    netcard->cmds_error = 0;

    return true;
}

/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dnetcard_info(token_t *parm, device_t *dev)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    connection_t *conn;
    for_each(netcard->listening_conns, conn, connection_t)
    {
        char listen_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &conn->src_addr.sin_addr, listen_address, INET_ADDRSTRLEN);

        printf("Listening on %s:%d\n", listen_address, (uint16_t) ntohs(conn->src_addr.sin_port));
    }
    printf("Opened connections:...\n");
    return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool dnetcard_stat(token_t *parm, device_t *dev)
{
    printf("Network card statistics\n");
    return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool dnetcard_listen(token_t *parm, device_t *dev)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    uint64_t _port = parm_uint_next(&parm);

    // parse optional parameter visibility
    bool public_addr = false;
    if (parm_type(parm) != tt_end) {
        const char *visibility = parm_str_next(&parm);
        if (strcmp(visibility, "public") == 0) {
            public_addr = true;
        } else if (strcmp(visibility, "local") == 0) {
            public_addr = false;
        } else {
            error("Unknown visibility string %s. Expected 'local' or 'public'.", visibility);
            return false;
        }
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(public_addr ? INADDR_ANY : INADDR_LOOPBACK);

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        io_error("Error creating socket");
        return false;
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        io_error("Error setting socket option");
        return false;
    }

    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        io_error("Error binding address");
        return false;
    }

    if (listen(sockfd, SOMAXCONN) == -1) {
        io_error("Error listening on socket");
        return false;
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        io_error("Error setting fd to non-blocking");
        return false;
    }

    connection_t *new_conn = malloc(sizeof(connection_t));

    new_conn->socket = sockfd;
    new_conn->src_addr.sin_family = addr.sin_family;
    new_conn->src_addr.sin_port = addr.sin_port;
    new_conn->src_addr.sin_addr.s_addr = addr.sin_addr.s_addr;
    item_init(&new_conn->item);
    list_append(&netcard->listening_conns, &new_conn->item);
    netcard->listening_count++;

    return true;
}

/** Dispose network card
 *
 * @param dev Device pointer
 *
 */
static void dnetcard_done(device_t *dev)
{
    // todo free txbuffer
}

/** Read command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void dnetcard_read32(unsigned int procno, device_t *dev, ptr36_t addr,
        uint32_t *val)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    /* Read internal registers */
    switch (addr - netcard->addr) {
    case REGISTER_TX_ADDR_LO:
        *val = (uint32_t) (netcard->tx_ptr & UINT32_C(0xffffffff));
        break;
    case REGISTER_TX_ADDR_HI:
        *val = (uint32_t) (netcard->tx_ptr >> 32);
        break;
    case REGISTER_RX_ADDR_LO:
        *val = (uint32_t) (netcard->rx_ptr & UINT32_C(0xffffffff));
        break;
    case REGISTER_RX_ADDR_HI:
        *val = (uint32_t) (netcard->rx_ptr >> 32);
        break;
    case REGISTER_STATUS:
        *val = netcard->status;
        break;
    }
}

/** Write command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void dnetcard_write32(unsigned int procno, device_t *dev, ptr36_t addr,
        uint32_t val)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    switch (addr - netcard->addr) {
    case REGISTER_TX_ADDR_LO:
        netcard->tx_ptr &= ~((ptr36_t) UINT32_C(0xffffffff));
        netcard->tx_ptr |= val;
        break;
    case REGISTER_TX_ADDR_HI:
        netcard->tx_ptr &= (ptr36_t) UINT32_C(0xffffffff);
        netcard->tx_ptr |= ((ptr36_t) val) << 32;
        break;
    case REGISTER_RX_ADDR_LO:
        if (netcard->status & STATUS_RECEIVE) {
            netcard->status |= STATUS_INT_ERR;
            cpu_interrupt_up(NULL, netcard->intno);
            netcard->ig = true;
            netcard->intrcount++;
            netcard->cmds_error++;
            return;
        }

        netcard->rx_ptr &= ~((ptr36_t) UINT32_C(0xffffffff));
        netcard->rx_ptr |= val;
        break;
    case REGISTER_RX_ADDR_HI:
        if (netcard->status & STATUS_RECEIVE) {
            netcard->status |= STATUS_INT_ERR;
            cpu_interrupt_up(NULL, netcard->intno);
            netcard->ig = true;
            netcard->intrcount++;
            netcard->cmds_error++;
            return;
        }

        netcard->rx_ptr &= (ptr36_t) UINT32_C(0xffffffff);
        netcard->rx_ptr |= ((ptr36_t) val) << 32;
        break;

    case REGISTER_COMMAND:
        /* Remove unused bits */
        netcard->command = val & COMMAND_MASK;

        if (netcard->command & COMMAND_INT_TX_ACK) {
            netcard->status &= ~STATUS_INT_TX;
        }

        if (netcard->command & COMMAND_INT_RX_ACK) {
            netcard->status &= ~STATUS_INT_RX;
        }

        if (netcard->command & COMMAND_INT_ERR_ACK) {
            netcard->status &= ~STATUS_INT_ERR;
        }

        if (!(netcard->status & (STATUS_INT_TX | STATUS_INT_RX | STATUS_INT_ERR))) {
            netcard->ig = false;
            cpu_interrupt_down(NULL, netcard->intno);
        }

        if (netcard->status & STATUS_INT_ERR) {
            return;
        }

        if (netcard->command & COMMAND_RECEIVE) {
            if (netcard->status & STATUS_RECEIVE) {
                netcard->status &= ~STATUS_RECEIVE;
                netcard->rx_action = ACTION_NONE;
            } else {
                netcard->status |= STATUS_RECEIVE;
            }
        }

        if ((netcard->command & COMMAND_SEND) && netcard->tx_action != ACTION_NONE) {
            /* Command in progress */
            netcard->status |= STATUS_INT_ERR;
            cpu_interrupt_up(NULL, netcard->intno);
            netcard->ig = true;
            netcard->intrcount++;
            netcard->cmds_error++;
            return;
        }

        /* Send command */
        if (netcard->command & COMMAND_SEND) {
            netcard->tx_action = ACTION_PROCESS;
            netcard->tx_cnt = 0;
        }

        break;
    }
}

/** Network card implementation
 *
 * @param dev Device pointer
 *
 */
static void dnetcard_step(device_t *dev)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    switch (netcard->tx_action) {
    case ACTION_PROCESS:
        netcard->txbuffer[netcard->tx_cnt] = physmem_read32(-1 /*NULL*/, netcard->tx_ptr, true);

        /* Next word */
        netcard->tx_ptr += sizeof(uint32_t);
        netcard->tx_cnt++;

        if (netcard->tx_cnt == IP_PACKET_SIZE / sizeof(uint32_t)) {
            if (!dnetcard_send_packet(netcard)) {
                netcard->status |= STATUS_INT_ERR;
            }

            netcard->tx_action = ACTION_NONE;
            netcard->status |= STATUS_INT_TX;
            cpu_interrupt_up(NULL, netcard->intno);
            netcard->ig = true;
            netcard->intrcount++;
            netcard->packets_tx++;
        }
        break;
    default:
        break;
    }

    switch (netcard->rx_action) {
    case ACTION_PROCESS:
        physmem_write32(-1 /*NULL*/, netcard->rx_ptr, netcard->rxbuffer[netcard->rx_cnt], true);

        /* Next word */
        netcard->rx_ptr += sizeof(uint32_t);
        netcard->rx_cnt++;

        if (netcard->rx_cnt == IP_PACKET_SIZE / sizeof(uint32_t)) {
            netcard->rx_action = ACTION_NONE;
            netcard->status |= STATUS_INT_RX;
            cpu_interrupt_up(NULL, netcard->intno);
            netcard->ig = true;
            netcard->intrcount++;
            netcard->packets_rx++;
        }
        break;
    default:
        break;
    }
}

/** Network card listen for incoming data
 *
 * @param dev Device pointer
 *
 */
static void dnetcard_step4k(device_t *dev)
{
    netcard_data_s *netcard = (netcard_data_s *) dev->data;

    if (!(netcard->status & STATUS_RECEIVE)) {
        return;
    }

    /* Check listening ports for incomming connections */
    struct pollfd *listen_fds = malloc(netcard->listening_count * sizeof(struct pollfd));

    {
        connection_t *conn;
        int i = 0;
        for_each(netcard->listening_conns, conn, connection_t)
        {
            listen_fds[i].fd = conn->socket;
            listen_fds[i].events = POLLIN;
            i++;
        }
    }

    int listen_time = 0;
    if (poll(listen_fds, netcard->listening_count, listen_time) > 0) {
        int listen_fd;
        for (size_t i = 0; i < netcard->listening_count; i++) {
            if (listen_fds[i].revents & POLLIN) {
                listen_fd = listen_fds[i].fd;
                break;
            }
        }

        connection_t *listen_conn;
        for_each(netcard->listening_conns, listen_conn, connection_t)
        {
            if (listen_conn->socket == listen_fd) {
                dnetcard_create_incoming_connection(netcard, listen_conn);
                break;
            }
        }
    }

    free(listen_fds);

    /* List opened connections and read data from one */
    struct pollfd *fds = malloc(netcard->opened_count * sizeof(struct pollfd));

    connection_t *conn;
    int i = 0;
    for_each(netcard->opened_conns, conn, connection_t)
    {
        fds[i].fd = conn->socket;
        fds[i].events = POLLIN;
        i++;
    }

    int timeout = 0;
    int ret = poll(fds, netcard->opened_count, timeout);
    if (ret > 0) {
        int read_fd;
        for (size_t i = 0; i < netcard->opened_count; i++) {
            if (fds[i].revents & POLLIN) {
                read_fd = fds[i].fd;
                break;
            } else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dnetcard_close_connection(netcard, fds[i].fd);
                free(fds);
                return;
            }
        }
        connection_t *read_conn;
        for_each(netcard->opened_conns, read_conn, connection_t)
        {
            if (read_conn->socket == read_fd) {
                if (dnetcard_receive_packet(netcard, read_conn)) {
                    netcard->rx_action = ACTION_PROCESS;
                    netcard->rx_cnt = 0;
                    netcard->status &= ~STATUS_RECEIVE;
                } else {
                    dnetcard_close_connection(netcard, read_fd);
                }
                break;
            }
        }
    }

    free(fds);
}

cmd_t dnetcard_cmds[] = {
    { "init",
            (fcmd_t) dnetcard_init,
            DEFAULT,
            DEFAULT,
            "Initialization",
            "Initialization",
            REQ STR "name/network card name" NEXT
                    REQ INT "addr/register address" NEXT
                            REQ INT "intno/interrupt number within 0..6" END },
    { "help",
            (fcmd_t) dev_generic_help,
            DEFAULT,
            DEFAULT,
            "Display help",
            "Display help",
            OPT STR "cmd/command name" END },
    { "info",
            (fcmd_t) dnetcard_info,
            DEFAULT,
            DEFAULT,
            "Configuration information",
            "Configuration information",
            NOCMD },
    { "stat",
            (fcmd_t) dnetcard_stat,
            DEFAULT,
            DEFAULT,
            "Statistics",
            "Statistics",
            NOCMD },
    { "listen",
            (fcmd_t) dnetcard_listen,
            DEFAULT,
            DEFAULT,
            "Listen on port",
            "Listen on port",
            REQ INT "port" NEXT
                    OPT STR "visibility - local (default) or public" END },
    LAST_CMD
};

device_type_t dnetcard = {
    .nondet = true,

    /* Type name and description */
    .name = "dnetcard",
    .brief = "Network card simulation",
    .full = "TODO description",

    /* Functions */
    .done = dnetcard_done,
    .step = dnetcard_step,
    .step4k = dnetcard_step4k,
    .read32 = dnetcard_read32,
    .write32 = dnetcard_write32,

    /* Commands */
    .cmds = dnetcard_cmds

};
