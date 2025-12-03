/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  String constants
 *
 */

#include <errno.h>
#include <string.h>

#include "../config.h"
#include "main.h"
#include "text.h"

/*
 * The most frequently messages for user.
 * Device implementation should prefer these.
 */
const char *const txt_file_read_err = "Could not read file";
const char *const txt_filename_expected = "File name expected";
const char *const txt_file_create_err = "Could not create file";
const char *const txt_file_write_err = "Could not write to file";
const char *const txt_file_map_fail = "File map fail";
const char *const txt_devname_expected = "Device name expected";
const char *const txt_duplicate_devname = "Duplicate device name";
const char *const txt_devaddr_expected = "Device address expected";
const char *const txt_devaddr_error = "Device address error (4-byte alignment expected)";
const char *const txt_no_more_parms = "No more parameters allowed";
const char *const txt_not_en_mem = "Not enough memory for device inicialization";
const char *const txt_intnum_expected = "Interrupt number expected";
const char *const txt_intnum_range = "Interrupt number out of range 0..6";
const char *const txt_cmd_expected = "Command expected";
const char *const txt_unknown_cmd = "Unknown command";

const char *txt_exc[] = {
    /* 0 */
    "Interrupt",
    "TLB modification",
    "TLB (load or instruction fetch)",
    "TLB (store)",
    "Address error (load or instruction fetch)",
    "Address error (store)",
    "Bus error (instruction fetch)",
    "Bus error (data reference: load or store)",
    /* 8 */
    "Syscall",
    "Breakpoint",
    "Reserved instruction",
    "Coprocessor Unusable",
    "Arithmetic Overflow",
    "Trap",
    "Virtual Coherency instruction",
    "Floating-Point",
    /* 16 */
    "Exception 16",
    "Exception 17",
    "Exception 18",
    "Exception 19",
    "Exception 20",
    "Exception 21",
    "Exception 22",
    "Reference to WatchHi/WatchLo address",
    /* 24 */
    "Exception 24",
    "Exception 25",
    "Exception 26",
    "Exception 27",
    "Exception 28",
    "Exception 29",
    "Exception 30",
    "Virtual Coherency data"
};

const char txt_version[] = "MSIM version " PACKAGE_VERSION "\nCopyright (c) 2000-2024 MSIM authors (see Git repo for list of authors)\nLicenced under GPLv2.\n";

const char txt_help[] = "  -V, --version               display version info\n"
                        "  -c, --config=file_name      configuration file name\n"
                        "  -i, --interactive           enter interactive mode\n"
                        "  -t, --trace                 enter trace mode\n"
                        "  -g, --remote-gdb=port       enter gdb mode\n"
                        "  -d, --dap[port]            enter DAP mode (default: 10505)\n"
                        "  -n, --non-deterministic     enable non-deterministic behaviour\n"
                        "  -X, --no-extra-instructions disable MSIM-specific instructions\n";

const char hexchar[] = "0123456789abcdef";
