#ifndef _DRIVERS_DNETCARD_H_
#define _DRIVERS_DNETCARD_H_

#include <types.h>

/* Status flags */
#define STATUS_RECEIVE 0x02 /* Ready for receiving */
#define STATUS_INT_TX 0x04 /* Tx interrupt pending */
#define STATUS_INT_RX 0x08 /* Rx interrupt pending */
#define STATUS_INT_ERR 0x10 /* Error interrupt pending */

/* Command flags */
#define COMMAND_SEND 0x01 /**< Send packet */
#define COMMAND_RECEIVE 0x02 /**< Set receiving on/off */
#define COMMAND_INT_TX_ACK 0x04 /**< Tx interrupt acknowledge */
#define COMMAND_INT_RX_ACK 0x08 /**< Rx interrupt acknowledge */
#define COMMAND_INT_ERR_ACK 0x10 /* Error interrupt acknowledge */

#define NETCARD_ADDRESS (0x91000000)

typedef struct {
    uint32_t tx_addr_lo;
    uint32_t tx_addr_hi;
    uint32_t rx_addr_lo;
    uint32_t rx_addr_hi;
    uint32_t status_command;
    uint32_t ip_address;
} netcard_t;

#endif
