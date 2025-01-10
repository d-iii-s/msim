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

#include "dnetcard.h"

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
}

/** Network card implementation
 *
 * @param dev Device pointer
 *
 */
static void dnetcard_step4k(device_t *dev)
{
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
    .step = dnetcard_step4k,
    .read32 = dnetcard_read32,
    .write32 = dnetcard_write32,

    /* Commands */
    .cmds = dnetcard_cmds

};
