/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Reading and executing commands
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "assert.h"
#include "cmd.h"
#include "debug/breakpoint.h"
#include "debug/debug.h"
#include "device/cpu/mips_r4000/cpu.h"
#include "device/cpu/mips_r4000/debug.h"
#include "device/cpu/riscv_rv32ima/cpu.h"
#include "device/cpu/riscv_rv32ima/debug.h"
#include "device/cpu/superh_sh2e/cpu.h"
#include "device/cpu/superh_sh2e/debug.h"
#include "device/device.h"
#include "env.h"
#include "fault.h"
#include "main.h"
#include "utils.h"

static cmd_t *system_cmds;

typedef enum {
    command_name,
    device_name
} gen_type_t;

static gen_type_t gen_type = command_name;

/** Add command implementation
 *
 * Add memory, devices, etc.
 *
 * Device name should not be the same as
 * a command name and there should not be
 * another device with a same name.
 *
 */
static bool system_add(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    const char *device_type = parm_str(parm);
    parm_next(&parm);
    const char *device_name = parm_str(parm);

    /*
     * Check for conflicts between
     * the device name and a command name
     */

    if (cmd_find(device_name, system_cmds, NULL) == CMP_HIT) {
        error("Device name \"%s\" is in conflict with a command name",
                device_name);
        return false;
    }

    if (dev_by_name(device_name)) {
        error("Device name \"%s\" already added", device_name);
        return false;
    }

    /* Allocate device */
    device_t *dev = alloc_device(device_type, device_name);
    if (!dev) {
        return false;
    }

    /* Call device inicialization */
    if (!cmd_run_by_name("init", parm, dev->type->cmds, dev)) {
        free_device(dev);
        return false;
    }

    /* Add into the device list */
    add_device(dev);
    return true;
}

/** Continue command implementation
 *
 * Continue simulation.
 *
 */
static bool system_continue(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    machine_interactive = false;
    return true;
}

/* Step command implementation
 *
 * Execute a given count of instructions.
 *
 */
static bool system_step(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    switch (parm_type(parm)) {
    case tt_end:
        stepping = 1;
        machine_interactive = false;
        break;
    case tt_uint:
        stepping = parm_uint(parm);
        machine_interactive = false;
        break;
    default:
        intr_error("Unexpected parameter type");
        return false;
    }

    return true;
}

/** Set command implementation
 *
 * Set configuration variable.
 *
 */
static bool system_set(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    return env_cmd_set(parm);
}

/** Unset command implementation
 *
 * Unset configuration variable.
 *
 */
static bool system_unset(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    return env_cmd_unset(parm);
}

/** Dump instructions command implementation
 *
 * Dump instructions.
 *
 */
static bool system_dumpins(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    char *_cpu = parm_str_next(&parm);

    bool is_rv = strcmp(_cpu, "rv") == 0;
    bool is_r4k = strcmp(_cpu, "r4k") == 0;
    bool is_sh2e = strcmp(_cpu, "sh2e") == 0;

    if (!is_rv && !is_r4k && !is_sh2e) {
        error("Unknown CPU type (supported types: r4k, rv, sh2e)");
        return false;
    }

    // TODO Instruction size depends on the CPU!
    const unsigned insn_sz = is_sh2e ? sizeof(sh2e_insn_t) : (is_rv ? sizeof(rv_instr_t) : sizeof(r4k_instr_t));

    // TODO Alignment depends on instruction size.
    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), insn_sz);
    if (!phys_range(_addr)) {
        error("Physical address out of range");
        return false;
    }

    uint64_t _cnt = parm_uint(parm);
    if (!phys_range(_cnt)) {
        error("Count out of physical memory range");
        return false;
    }

    if (!phys_range(_addr + _cnt * insn_sz)) {
        error("Count exceeds physical memory range");
        return false;
    }

    ptr36_t addr;
    len36_t cnt;

    // TODO This should be handled by CPU devices.
    // TODO Memory reads should be handled by the CPU devices to account for endianness.
    for (addr = (ptr36_t) _addr, cnt = (len36_t) _cnt; cnt > 0;
            addr += insn_sz, cnt--) {

        if (is_r4k) {
            r4k_instr_t instr;
            instr.val = physmem_read32(-1, addr, false);
            r4k_idump_phys(addr, instr);

        } else if (is_rv) {
            rv_instr_t instr;
            instr.val = physmem_read32(-1, addr, false);
            rv32_idump_phys(addr, instr);
        } else if (is_sh2e) {
            sh2e_dump_insn_phys(addr);
        }
    }

    return true;
}

/** Dump devices command implementation
 *
 * Dump configured devices.
 *
 */
static bool system_dumpdev(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    dbg_print_devices(DEVICE_FILTER_ALL);
    return true;
}

/** Dump physical memory blocks command implementation
 *
 * Dump configured physical memory blocks.
 *
 */
static bool system_dumpphys(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    dbg_print_devices(DEVICE_FILTER_MEMORY);
    return true;
}

/** Break command implementation
 *
 */
static bool system_break(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    uint64_t addr = parm_uint_next(&parm);
    uint64_t size = parm_uint_next(&parm);
    const char *rw = parm_str(parm);

    if (!phys_range(addr)) {
        error("Physical address out of range");
        return false;
    }

    if (!phys_range(size)) {
        error("Size out of physical memory range");
        return false;
    }

    if (!phys_range(addr + size)) {
        error("Size exceeds physical memory range");
        return false;
    }

    access_filter_t access_flags = ACCESS_FILTER_NONE;

    if (strchr(rw, 'r') != NULL) {
        access_flags |= ACCESS_READ;
    }

    if (strchr(rw, 'w') != NULL) {
        access_flags |= ACCESS_WRITE;
    }

    if (access_flags == ACCESS_FILTER_NONE) {
        error("Read or write access must be specified");
        return false;
    }

    physmem_breakpoint_add((ptr36_t) addr, (len36_t) size,
            BREAKPOINT_KIND_SIMULATOR, access_flags);

    return true;
}

/** Dump breakpoints command implementation
 *
 */
static bool system_dumpbreak(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    physmem_breakpoint_print_list();
    return true;
}

/** Remove breakpoint command implementation
 *
 */
static bool system_rembreak(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    uint64_t addr = parm_uint(parm);

    if (!phys_range(addr)) {
        error("Physical address out of range");
        return false;
    }

    if (!physmem_breakpoint_remove(addr)) {
        error("Unknown breakpoint");
        return false;
    }

    return true;
}

/** Stat command implementation
 *
 * Print simulator statistics.
 *
 */
static bool system_stat(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    dbg_print_devices_stat(DEVICE_FILTER_ALL);
    return true;
}

/** Dump memory command implementation
 *
 * Dump physical memory.
 *
 */
static bool system_dumpmem(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
    uint64_t _cnt = parm_uint(parm);

    if (!phys_range(_addr)) {
        error("Physical address out of range");
        return false;
    }

    if (!phys_range(_cnt)) {
        error("Count out of physical memory range");
        return false;
    }

    if (!phys_range(_addr + _cnt * 4)) {
        error("Count exceeds physical memory range");
        return false;
    }

    ptr36_t addr;
    len36_t cnt;
    len36_t i;

    for (addr = (ptr36_t) _addr, cnt = (len36_t) _cnt, i = 0;
            i < cnt; addr += 4, i++) {
        if ((i & 0x03U) == 0) {
            printf("  %#011" PRIx64 "   ", addr);
        }

        uint32_t val = physmem_read32(-1, addr, false);
        printf("%08" PRIx32 " ", val);

        if ((i & 0x03U) == 3) {
            printf("\n");
        }
    }

    if (i != 0) {
        printf("\n");
    }

    return true;
}

/** Quit command implementation
 *
 * Quit MSIM immediately.
 *
 */
static bool system_quit(token_t *parm, void *data)
{
    ASSERT(parm != NULL);
    machine_interactive = false;
    machine_halt = true;
    return true;
}

/** Echo command implementation
 *
 * Print the user text on the screen.
 *
 */
static bool system_echo(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    while (parm_type(parm) != tt_end) {
        switch (parm_type(parm)) {
        case tt_str:
            printf("%s", parm_str(parm));
            break;
        case tt_uint:
            printf("%" PRIu64, parm_uint(parm));
            break;
        default:
            intr_error("Unexpected parameter type");
            return false;
        }

        parm_next(&parm);
        if (parm_type(parm) != tt_end) {
            printf(" ");
        }
    }

    printf("\n");
    return true;
}

/** Help command implementation
 *
 * Print the help.
 *
 */
static bool system_help(token_t *parm, void *data)
{
    ASSERT(parm != NULL);

    cmd_print_extended_help(system_cmds, parm);
    return true;
}

/** Interprets the command line.
 *
 * Line is terminated by '\0' or '\n'.
 *
 */
bool interpret(const char *str)
{
    ASSERT(str != NULL);

    /* Parse input */
    token_t *parm = parm_parse(str);

    if (parm_type(parm) == tt_end) {
        parm_delete(parm);
        return true;
    }

    if (parm_type(parm) != tt_str) {
        error("Command name expected");
        parm_delete(parm);
        return true;
    }

    const char *name = parm_str(parm);
    bool ret;
    device_t *dev = dev_by_name(name);

    if (dev) {
        /* Device command */
        parm_next(&parm);
        ret = cmd_run_by_parm(parm, dev->type->cmds, dev);
    } else {
        /* System command */
        ret = cmd_run_by_parm(parm, system_cmds, NULL);
    }

    parm_delete(parm);
    return ret;
}

/** Run initial script uploaded in the memory
 *
 */
static bool setup_apply(const char *buf)
{
    ASSERT(buf != NULL);

    size_t lineno = 1;

    while ((*buf) && (!machine_halt)) {
        set_lineno(lineno);
        if (!interpret(buf)) {
            return false;
        }

        /* Move to the next line */
        while ((*buf) && (*buf != '\n')) {
            buf++;
        }

        if (*buf == '\n') {
            buf++;
        }

        lineno++;
    }

    return true;
}

/** Interpret configuration file
 *
 */
void script(void)
{
    if (!config_file) {
        /* Check for variable MSIMCONF */
        config_file = getenv("MSIMCONF");
        if (!config_file) {
            config_file = "msim.conf";
        }
    }

    /* Open configuration file */
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            alert("Configuration file \"%s\" not found, skipping",
                    config_file);
        } else {
            io_die(ERR_IO, config_file);
        }

        machine_interactive = true;
        return;
    }

    if (check_isdir(file)) {
        alert("Path \"%s\" is a directory, skipping", config_file);
        safe_fclose(file, config_file);
        machine_interactive = true;
        return;
    }

    set_script(config_file);

    string_t str;
    string_init(&str);
    string_fread(&str, file);

    safe_fclose(file, config_file);

    if (!setup_apply(str.str)) {
        die(ERR_INIT, "Error in configuration file");
    }

    unset_script();
    string_done(&str);
}

/** Generate a list of device types
 *
 */
static char *generator_devtype(token_t *parm, const void *data,
        unsigned int level)
{
    ASSERT(parm != NULL);
    ASSERT((parm_type(parm) == tt_str) || (parm_type(parm) == tt_end));

    const char *str;
    static uint32_t last_device_order = 0;

    if (level == 0) {
        last_device_order = 0;
    }

    if (parm_type(parm) == tt_str) {
        str = dev_type_by_partial_name(parm_str(parm), &last_device_order);
    } else {
        str = dev_type_by_partial_name("", &last_device_order);
    }

    return str ? safe_strdup(str) : NULL;
}

/** Generate a list of installed device names
 *
 */
static char *generator_devname(token_t *parm, const void *data,
        unsigned int level)
{
    ASSERT(parm != NULL);
    ASSERT((parm_type(parm) == tt_str) || (parm_type(parm) == tt_end));

    const char *str;
    static device_t *dev;

    if (level == 0) {
        dev = NULL;
    }

    if (parm_type(parm) == tt_str) {
        str = dev_by_partial_name(parm_str(parm), &dev);
    } else {
        str = dev_by_partial_name("", &dev);
    }

    return str ? safe_strdup(str) : NULL;
}

/** Generate a list of commands and device names
 *
 * @param unused_data Always NULL.
 *
 */
static char *generator_system_cmds_and_device_names(token_t *parm,
        const void *unused_data, unsigned int level)
{
    ASSERT(parm != NULL);

    const char *str = NULL;

    if (level == 0) {
        gen_type = command_name;
    }

    if (gen_type == command_name) {
        str = generator_cmd(parm, system_cmds + 1, level);
        if (!str) {
            gen_type = device_name;
            level = 0;
        }
    }

    if (gen_type == device_name) {
        str = generator_devname(parm, NULL, level);
    }

    return str ? safe_strdup(str) : NULL;
}

/** Add command find generator
 *
 */
static gen_t system_add_find_generator(token_t **parm, const cmd_t *cmd,
        const void **data)
{
    ASSERT(parm != NULL);
    ASSERT(*parm != NULL);
    ASSERT(cmd != NULL);
    ASSERT(data != NULL);
    ASSERT(*data == NULL);

    /*
     * If this does not look like an add command, we bail out.
     */
    if (!((parm_type(*parm) == tt_str) && (strcmp(parm_str(*parm), "add") == 0))) {
        return NULL;
    }

    /*
     * We skip the first token - "add" command name - and check that user
     * supplied only one more parameter (we do not auto-complete for device
     * properties).
     */
    token_t *next = parm_next(parm);
    if (parm_last(next)) {
        return generator_devtype;
    }

    return NULL;
}

/** Set command find generator
 *
 */
static gen_t system_set_find_generator(token_t **parm, const cmd_t *cmd,
        const void **data)
{
    ASSERT(parm != NULL);
    ASSERT(*parm != NULL);
    ASSERT(cmd != NULL);
    ASSERT(data != NULL);
    ASSERT(*data == NULL);

    if (parm_type(*parm) == tt_str) {
        unsigned int res;

        /* Look up for a variable name */
        if (parm_last(*parm)) {
            /* There is a completion possible */
            res = env_cnt_partial_varname(parm_str(*parm));
        } else {
            /* Exactly one match is allowed */
            res = 1;
        }

        if (res == 1) {
            /* Variable fit by partial name */
            if (parm_last(*parm)) {
                return generator_env_name;
            }

            var_type_t type;

            if (env_check_varname(parm_str(*parm), &type)) {
                parm_next(parm);
                if (parm_type(*parm) == tt_str) {
                    if (!strcmp(parm_str(*parm), "=")) {
                        /* Search for value */
                        parm_next(parm);

                        if ((parm_type(*parm) == tt_str) && (type == vt_bool)) {
                            if (parm_last(*parm)) {
                                return generator_env_booltype;
                            }
                        }
                    } else if (!parm_str(*parm)[0]) {
                        return generator_equal_char;
                    }
                }
            }
        }
    } else {
        /* Multiple hit */
        if (parm_last(*parm)) {
            return generator_env_name;
        }
    }

    return NULL;
}

/** Unset command find generator
 *
 */
static gen_t system_unset_find_generator(token_t **parm, const cmd_t *cmd,
        const void **data)
{
    ASSERT(parm != NULL);
    ASSERT(*parm != NULL);
    ASSERT(cmd != NULL);
    ASSERT(data != NULL);
    ASSERT(*data == NULL);

    if (parm_type(*parm) == tt_str) {
        /* Look up for a variable name */
        unsigned int res = env_cnt_partial_varname(parm_str(*parm));

        if (res > 0) {
            /* Partially fit by partial name */
            if (parm_last(*parm)) {
                return generator_bool_envname;
            }
        }
    }

    return NULL;
}

/** Look up for the completion generator
 *
 * The command is specified by the first parameter.
 *
 */
gen_t find_completion_generator(token_t **parm, const void **data)
{
    ASSERT(parm != NULL);
    ASSERT(*parm != NULL);
    ASSERT(data != NULL);
    ASSERT(*data == NULL);

    if (parm_last(*parm)) {
        return generator_system_cmds_and_device_names;
    }

    /* Check if the first token is a string */
    if (parm_type(*parm) != tt_str) {
        return NULL;
    }

    char *user_text = parm_str(*parm);

    /* Find a command */
    const cmd_t *cmd;
    cmd_find_res_t res = cmd_find(user_text, system_cmds + 1, &cmd);

    switch (res) {
    case CMP_NO_HIT:
    case CMP_PARTIAL_HIT:
        /*
         * Unknown command.
         *
         * If the user has written only the first part of the command,
         * then use device names completion. If there is also a second
         * part written and the first part leads just to one device name,
         * use commands for that device as completion.
         */
        if (parm_last(*parm)) {
            return generator_system_cmds_and_device_names;
        }

        device_t *last_device = NULL;
        size_t devices_count = dev_count_by_partial_name(user_text,
                &last_device);

        if (devices_count == 1) {
            parm_next(parm);
            return dev_find_generator(parm, last_device, data);
        }

        break;
    case CMP_MULTIPLE_HIT:
    case CMP_HIT:
        /* Default system generator */
        if (parm_last(*parm)) {
            return generator_system_cmds_and_device_names;
        }

        if (res == CMP_MULTIPLE_HIT) {
            /* Input error */
            break;
        }

        /* Continue to the next generator if possible */
        if (cmd->find_gen) {
            return cmd->find_gen(parm, cmd, data);
        }

        break;
    default:
        intr_error("Unexpected compare result");
    }

    return NULL;
}

/** Main command structure
 *
 * All system commands are defined here.
 *
 */
static cmd_t system_cmds_array[] = {
    { "init",
            NULL, /* hardwired */
            DEFAULT,
            DEFAULT,
            "",
            "",
            NOCMD },
    { "add",
            system_add,
            system_add_find_generator,
            DEFAULT,
            "Add a new device into the system",
            "Add a new device into the system",
            REQ STR "type/Device type" NEXT
                    REQ STR "name/Device name" CONT },
    { "dumpmem",
            system_dumpmem,
            DEFAULT,
            DEFAULT,
            "Dump words from physical memory",
            "Dump words from physical memory",
            REQ INT "addr/memory address" NEXT
                    REQ INT "cnt/count" END },
    { "dumpins",
            system_dumpins,
            DEFAULT,
            DEFAULT,
            "Dump instructions from physical memory",
            "Dump instructions from physical memory",
            REQ STR "cpu" NEXT
                    REQ INT "addr/memory address" NEXT
                            REQ INT "cnt/count" END },
    { "dumpdev",
            system_dumpdev,
            DEFAULT,
            DEFAULT,
            "Dump installed devices",
            "Dump installed devices",
            NOCMD },
    { "dumpphys",
            system_dumpphys,
            DEFAULT,
            DEFAULT,
            "Dump installed physical memory blocks",
            "Dump installed physical memory blocks",
            NOCMD },
    { "break",
            system_break,
            DEFAULT,
            DEFAULT,
            "Add a new physical memory breakpoint",
            "Add a new physical memory breakpoint",
            REQ INT "addr/memory address" NEXT
                    REQ INT "cnt/count" NEXT
                            REQ STR "type/Read or write breakpoint" END },
    { "dumpbreak",
            system_dumpbreak,
            DEFAULT,
            DEFAULT,
            "Dump physical memory breakpoints",
            "Dump physical memory breakpoints",
            NOCMD },
    { "rembreak",
            system_rembreak,
            DEFAULT,
            DEFAULT,
            "Remove a physical memory breakpoint",
            "Remove a physical memory breakpoint",
            REQ INT "addr/memory address" END },
    { "stat",
            system_stat,
            DEFAULT,
            DEFAULT,
            "Print system statistics",
            "Print system statistics",
            NOCMD },
    { "echo",
            system_echo,
            DEFAULT,
            DEFAULT,
            "Print user message",
            "Print user message",
            OPT VAR "text" CONT },
    { "continue",
            system_continue,
            DEFAULT,
            DEFAULT,
            "Continue simulation",
            "Continue simulation",
            NOCMD },
    { "step",
            system_step,
            DEFAULT,
            DEFAULT,
            "Simulate one or a specified number of instructions",
            "Simulate one or a specified number of instructions",
            OPT INT "cnt/intruction count" END },
    { "set",
            system_set,
            system_set_find_generator,
            DEFAULT,
            "Set enviroment variable",
            "Set enviroment variable",
            OPT STR "name/variable name" NEXT
                    OPT CON "=" NEXT
                            REQ VAR "val/value" END },
    { "unset",
            system_unset,
            system_unset_find_generator,
            DEFAULT,
            "Unset environment variable",
            "Unset environment variable",
            REQ STR "name/variable name" END },
    { "help",
            system_help,
            DEFAULT,
            DEFAULT,
            "Display help",
            "Display help",
            OPT STR "cmd/command name" END },
    { "quit",
            system_quit,
            DEFAULT,
            DEFAULT,
            "Exit MSIM",
            "Exit MSIM",
            NOCMD },
    LAST_CMD
};
static cmd_t *system_cmds = system_cmds_array;
