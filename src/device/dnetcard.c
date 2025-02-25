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

#include <stdio.h>
#include <netinet/ip.h>

#include "../fault.h"
#include "../physmem.h"
#include "../text.h"
#include "../utils.h"
#include "cpu/general_cpu.h"
#include "dnetcard.h"

/** Actions the network card is performing */
enum action_e {
    ACTION_NONE, /**< Network card is idle */
    ACTION_SEND, /**< Network card is sending a packet */
};

/* Register offsets */
#define REGISTER_ADDR_LO 0 /* Address (bits 0 .. 31) */
#define REGISTER_ADDR_HI 4 /* Address (bits 32 .. 35) */
#define REGISTER_STATUS 8 /* Status/commands */
#define REGISTER_COMMAND 8 /* Status/commands */
#define REGISTER_LIMIT 12 /* Size of register block */

/* Status flags */
#define STATUS_INT 0x04 /* Interrupt pending */
#define STATUS_ERROR 0x08 /* Command error*/
#define STATUS_MASK 0x0c /* Status mask */

/* Command flags */
#define COMMAND_SEND 0x01 /**< Send packet */
#define COMMAND_MASK 0x07 /* Command mask */

/* Constants */
// #define TX_BUFFER_SIZE ALIGN_UP(IP_MAXPACKET, 4)
#define TX_BUFFER_SIZE 128

/** Network card instance data structure */
typedef struct {
    uint32_t *txbuffer; /* Transfer buffer */

    /* Configuration */
    ptr36_t addr; /* Register address */
    unsigned int intno; /* Interrupt number */

    /* Registers */
    ptr36_t netcard_ptr; /* Current DMA pointer */
    uint32_t netcard_status; /* Network card status register */
    uint32_t netcard_command; /* Network card command register */

    /* Current action variables */
    enum action_e action; /* Action type */
    size_t cnt; /* Word counter */
    bool ig; /* Interrupt pending flag */

    /* Statistics */
    uint64_t intrcount; /* Number of interrupts */
    uint64_t cmds_send; /* Number of read commands */
    uint64_t cmds_error; /* Number of illegal commands */

} netcard_data_s;

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
    data->addr = addr;
    data->intno = _intno;
    data->txbuffer = safe_malloc(TX_BUFFER_SIZE);
    data->netcard_ptr = 0;
    data->netcard_status = 0;
    data->netcard_command = 0;
    data->action = ACTION_NONE;
    data->cnt = 0;
    data->ig = false;
    data->intrcount = 0;
    data->cmds_send = 0;
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
    case REGISTER_ADDR_LO:
        *val = (uint32_t) (data->netcard_ptr & UINT32_C(0xffffffff));
        break;
    case REGISTER_ADDR_HI:
        *val = (uint32_t) (data->netcard_ptr >> 32);
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
    case REGISTER_ADDR_LO:
        data->netcard_ptr &= ~((ptr36_t) UINT32_C(0xffffffff));
        data->netcard_ptr |= val;
        printf("addr set\n");
        break;
    case REGISTER_ADDR_HI:
        data->netcard_ptr &= (ptr36_t) UINT32_C(0xffffffff);
        data->netcard_ptr |= ((ptr36_t) val) << 32;
        break;

    case REGISTER_COMMAND:
        /* Remove unused bits */
        data->netcard_command = val & COMMAND_MASK;

        if ((data->netcard_command & COMMAND_SEND) && data->action != ACTION_NONE) {
            /* Command in progress */
            data->netcard_status = STATUS_INT | STATUS_ERROR;
            cpu_interrupt_up(NULL, data->intno);
            data->ig = true;
            data->intrcount++;
            data->cmds_error++;
            return;
        }

        /* Send command */
        if (data->netcard_command & COMMAND_SEND) {
            printf("send cmd\n");
            data->action = ACTION_SEND;
            data->cnt = 0;
            data->cmds_send++;
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

    switch (data->action) {
    case ACTION_SEND:
        data->txbuffer[data->cnt] = physmem_read32(-1 /*NULL*/, data->netcard_ptr, true);

        /* Next word */
        data->netcard_ptr += sizeof(uint32_t);
        data->cnt++;
        break;
    default:
        return;
    }

    if (data->cnt == TX_BUFFER_SIZE / sizeof(uint32_t)) {
        // todo send packet

        data->action = ACTION_NONE;
        data->netcard_status = STATUS_INT;
        cpu_interrupt_up(NULL, data->intno);
        data->ig = true;
        data->intrcount++;
    }
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
    .read32 = dnetcard_read32,
    .write32 = dnetcard_write32,

    /* Commands */
    .cmds = dnetcard_cmds

};
