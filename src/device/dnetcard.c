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

#include <poll.h>
#include <stdio.h>
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
#define IP_PACKET_SIZE ALIGN_UP(IP_MAXPACKET, 4)
#define IP_HEADER_LEN 0x5
#define TCP_HEADER_OFF 0x5
#define TX_BUFFER_SIZE IP_PACKET_SIZE
#define RX_BUFFER_SIZE IP_PACKET_SIZE

typedef struct {
    item_t item;
    struct sockaddr_in addr;
    int socket;
} connection_t;

/** Network card instance data structure */
typedef struct {
    uint32_t *txbuffer; /* Transfer buffer */
    uint32_t *rxbuffer; /* Receive buffer */
    list_t opened_conns;
    size_t conn_count;

    /* Configuration */
    ptr36_t addr; /* Register address */
    unsigned int intno; /* Interrupt number */

    /* Registers */
    ptr36_t netcard_tx_ptr; /* Current tx DMA pointer */
    ptr36_t netcard_rx_ptr; /* Current rx DMA pointer */
    uint32_t netcard_status; /* Network card status register */
    uint32_t netcard_command; /* Network card command register */

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

/** Create a connection
 *
 * @param data Network card instance data structure
 * @param addr Connection address structure
 *
 * @return Socket file descriptor, -1 on errors
 */
static int dnetcard_create_connection(netcard_data_s *data, struct sockaddr_in *addr)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        // todo signal error in status
        io_error("Failed to create socket");
        return -1;
    }

    if (connect(fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) == -1) {
        io_error("Failed to connect to address");
        close(fd);
        return -1;
    }

    connection_t *new_conn = malloc(sizeof(connection_t));
    new_conn->addr.sin_family = addr->sin_family;
    new_conn->addr.sin_addr = addr->sin_addr;
    new_conn->addr.sin_port = addr->sin_port;
    new_conn->socket = fd;
    item_init(&new_conn->item);
    list_append(&data->opened_conns, &new_conn->item);
    data->conn_count++;

    return fd;
}

/** Get an open connection socket
 *
 * @param data Network card instance data structure
 * @param addr Connection address structure
 *
 * @return File descriptor of the socket, -1 if connection does not exist
 */
static int dnetcard_get_connection(netcard_data_s *data, struct sockaddr_in *addr)
{
    connection_t *conn;
    for_each(data->opened_conns, conn, connection_t)
    {
        if (conn->addr.sin_addr.s_addr == addr->sin_addr.s_addr && conn->addr.sin_port == addr->sin_port) {
            return conn->socket;
        }
    }

    return -1;
}

/** Send packet stored in txbuffer
 *
 * @param data Network card instance data structure
 *
 */
static void dnetcard_send_packet(netcard_data_s *data)
{
    struct ip *ip_header = (struct ip *) data->txbuffer;
    struct tcphdr *tcp_header = (struct tcphdr *) (data->txbuffer + ip_header->ip_hl);

    struct sockaddr_in in = { 0 };
    in.sin_family = AF_INET;
    in.sin_port = tcp_header->th_dport;
    in.sin_addr = ip_header->ip_dst;

    int fd;

    if ((fd = dnetcard_get_connection(data, &in)) == -1) {
        if ((fd = dnetcard_create_connection(data, &in)) == -1) {
            return;
        }
    }

    char *packet_data = (char *) (tcp_header + tcp_header->th_off * sizeof(uint32_t));
    size_t data_size = ip_header->ip_len - ip_header->ip_hl - tcp_header->th_off;

    for (size_t i = 0; i < data_size; i++) {
        write(fd, packet_data + i, 1);
    }
}

static void dnetcard_receive_packet(netcard_data_s *data, connection_t *conn)
{
    struct ip *ip_header = (struct ip *) data->rxbuffer;
    ip_header->ip_hl = IP_HEADER_LEN;
    struct in_addr src_addr = { conn->addr.sin_addr.s_addr };
    ip_header->ip_src = src_addr;

    struct tcphdr *tcp_header = (struct tcphdr *) (data->rxbuffer + ip_header->ip_hl);
    tcp_header->th_off = TCP_HEADER_OFF;
    tcp_header->th_sport = conn->addr.sin_port;

    char *data_ptr = (char *) (tcp_header + tcp_header->th_off * sizeof(uint32_t));

    size_t data_size = read(conn->socket, data_ptr, IP_PACKET_SIZE - ip_header->ip_hl - tcp_header->th_off);

    ip_header->ip_len = ip_header->ip_hl + tcp_header->th_off + data_size;
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
    netcard_data_s *data = safe_malloc_t(netcard_data_s);
    dev->data = data;

    /* Initialization */
    list_init(&data->opened_conns);
    data->addr = addr;
    data->intno = _intno;
    data->txbuffer = safe_malloc(TX_BUFFER_SIZE);
    data->rxbuffer = safe_malloc(RX_BUFFER_SIZE);
    data->netcard_tx_ptr = 0;
    data->netcard_status = 0;
    data->netcard_command = 0;
    data->tx_action = ACTION_NONE;
    data->rx_action = ACTION_NONE;
    data->tx_cnt = 0;
    data->rx_cnt = 0;
    data->ig = false;
    data->intrcount = 0;
    data->packets_tx = 0;
    data->packets_rx = 0;
    data->cmds_error = 0;

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
    printf("Network card info\n");
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
    netcard_data_s *data = (netcard_data_s *) dev->data;

    /* Read internal registers */
    switch (addr - data->addr) {
    case REGISTER_TX_ADDR_LO:
        *val = (uint32_t) (data->netcard_tx_ptr & UINT32_C(0xffffffff));
        break;
    case REGISTER_TX_ADDR_HI:
        *val = (uint32_t) (data->netcard_tx_ptr >> 32);
        break;
    case REGISTER_RX_ADDR_LO:
        *val = (uint32_t) (data->netcard_rx_ptr & UINT32_C(0xffffffff));
        break;
    case REGISTER_RX_ADDR_HI:
        *val = (uint32_t) (data->netcard_rx_ptr >> 32);
        break;
    case REGISTER_STATUS:
        *val = data->netcard_status;
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
    netcard_data_s *data = (netcard_data_s *) dev->data;

    switch (addr - data->addr) {
    case REGISTER_TX_ADDR_LO:
        data->netcard_tx_ptr &= ~((ptr36_t) UINT32_C(0xffffffff));
        data->netcard_tx_ptr |= val;
        break;
    case REGISTER_TX_ADDR_HI:
        data->netcard_tx_ptr &= (ptr36_t) UINT32_C(0xffffffff);
        data->netcard_tx_ptr |= ((ptr36_t) val) << 32;
        break;
    case REGISTER_RX_ADDR_LO:
        if (data->netcard_status & STATUS_RECEIVE) {
            data->netcard_status = STATUS_INT_ERR;
            cpu_interrupt_up(NULL, data->intno);
            data->ig = true;
            data->intrcount++;
            data->cmds_error++;
            return;
        }

        data->netcard_rx_ptr &= ~((ptr36_t) UINT32_C(0xffffffff));
        data->netcard_rx_ptr |= val;
        break;
    case REGISTER_RX_ADDR_HI:
        if (data->netcard_status & STATUS_RECEIVE) {
            data->netcard_status = STATUS_INT_ERR;
            cpu_interrupt_up(NULL, data->intno);
            data->ig = true;
            data->intrcount++;
            data->cmds_error++;
            return;
        }

        data->netcard_rx_ptr &= (ptr36_t) UINT32_C(0xffffffff);
        data->netcard_rx_ptr |= ((ptr36_t) val) << 32;
        break;

    case REGISTER_COMMAND:
        /* Remove unused bits */
        data->netcard_command = val & COMMAND_MASK;

        if (data->netcard_command & COMMAND_INT_TX_ACK) {
            data->netcard_status &= ~STATUS_INT_TX;
        }

        if (data->netcard_command & COMMAND_INT_RX_ACK) {
            data->netcard_status &= ~STATUS_INT_RX;
        }

        if (data->netcard_command & COMMAND_INT_ERR_ACK) {
            data->netcard_status &= ~STATUS_INT_ERR;
        }

        if (!(data->netcard_status & (STATUS_INT_TX | STATUS_INT_RX | STATUS_INT_ERR))) {
            data->ig = false;
            cpu_interrupt_down(NULL, data->intno);
        }

        if (data->netcard_status & STATUS_INT_ERR) {
            return;
        }

        if (data->netcard_command & COMMAND_RECEIVE) {
            if (data->netcard_status & STATUS_RECEIVE) {
                data->netcard_status &= ~STATUS_RECEIVE;
                data->rx_action = ACTION_NONE;
            }
            data->netcard_status ^= STATUS_RECEIVE;
        }

        if ((data->netcard_command & COMMAND_SEND) && data->tx_action != ACTION_NONE) {
            /* Command in progress */
            data->netcard_status = STATUS_INT_ERR;
            cpu_interrupt_up(NULL, data->intno);
            data->ig = true;
            data->intrcount++;
            data->cmds_error++;
            return;
        }

        /* Send command */
        if (data->netcard_command & COMMAND_SEND) {
            data->tx_action = ACTION_PROCESS;
            data->tx_cnt = 0;
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
    netcard_data_s *data = (netcard_data_s *) dev->data;

    switch (data->tx_action) {
    case ACTION_PROCESS:
        data->txbuffer[data->tx_cnt] = physmem_read32(-1 /*NULL*/, data->netcard_tx_ptr, true);

        /* Next word */
        data->netcard_tx_ptr += sizeof(uint32_t);
        data->tx_cnt++;
        break;
    default:
        return;
    }

    switch (data->rx_action) {
    case ACTION_PROCESS:
        physmem_write32(-1 /*NULL*/, data->netcard_rx_ptr, data->rxbuffer[data->rx_cnt], true);

        /* Next word */
        data->netcard_rx_ptr += sizeof(uint32_t);
        data->rx_cnt++;
        break;
    default:
        break;
    }

    if (data->tx_cnt == IP_PACKET_SIZE / sizeof(uint32_t)) {
        dnetcard_send_packet(data);

        data->tx_action = ACTION_NONE;
        data->netcard_status = STATUS_INT_TX;
        cpu_interrupt_up(NULL, data->intno);
        data->ig = true;
        data->intrcount++;
        data->packets_tx++;
    }

    if (data->rx_cnt == IP_PACKET_SIZE / sizeof(uint32_t)) {
        data->rx_action = ACTION_NONE;
        data->netcard_status = STATUS_INT_RX;
        cpu_interrupt_up(NULL, data->intno);
        data->ig = true;
        data->intrcount++;
        data->packets_rx++;
    }
}

/** Network card listen for incoming data
 *
 * @param dev Device pointer
 *
 */
static void dnetcard_step4k(device_t *dev)
{
    netcard_data_s *data = (netcard_data_s *) dev->data;

    struct pollfd *fds = malloc(data->conn_count * sizeof(struct pollfd));

    // todo refactor, this is not nice
    // maybe use different data structure for opened connections?

    connection_t *conn;
    int i = 0;
    for_each(data->opened_conns, conn, connection_t)
    {
        fds[i].fd = conn->socket;
        fds[i].events = POLLIN;
        i++;
    }

    int timeout = 0;
    int ret = poll(fds, data->conn_count, timeout);
    if (ret > 0) {
        int read_fd;
        for (int i = 0; i < data->conn_count; i++) {
            if (fds[i].revents & POLLIN) {
                read_fd = fds[i].fd;
            }
        }
        connection_t *read_conn;
        for_each(data->opened_conns, read_conn, connection_t)
        {
            if (read_conn->socket == read_fd) {
                dnetcard_receive_packet(data, read_conn);
                break;
            }
        }

        data->rx_action = ACTION_PROCESS;
        data->rx_cnt = 0;
        data->netcard_status &= ~STATUS_RECEIVE;
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
