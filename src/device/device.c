/*
 * Copyright (c) 2001-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Device infrastructure
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "../assert.h"
#include "../env.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"
#include "dcycle.h"
#include "ddisk.h"
#include "device.h"
#include "dkeyboard.h"
#include "dnomem.h"
#include "dorder.h"
#include "dprinter.h"
#include "dr4kcpu.h"
#include "drvcpu.h"
#include "dsh2ecpu.h"
#include "dtime.h"
#include "mem.h"


/* Known device types. */
static device_type_t const * const device_types[] = {
    &dr4kcpu,
    &drvcpu,
    &dsh2ecpu,
    &dcycle,
    &drwm,
    &drom,
    &dprinter,
    &dorder,
    &dkeyboard,
    &dnomem,
    &ddisk,
    &dtime
};


/* List of all devices */
list_t device_list = LIST_INITIALIZER;


/** Search for device type by name.
 *
 * @param device_name Name, which will be set to created device.
 *
 * @return Pointer to the device type or NULL if the device
 * type cannot be found.
 */
static device_type_t const *
dev_type_by_name(char const * const type_name) {
    ASSERT(type_name != NULL);

    for (unsigned i = 0; i < array_len(device_types); i++) {
        device_type_t const * device_type = device_types[i];
        if (strcmp(type_name, device_type->name) == 0) {
            return device_type;
        }
    }

    // Not found.
    return NULL;
}


/** Search for device type and allocates device structure
 *
 * @param type_string Exact name of device type.
 * @param device_name Name, which will be set to created device.
 *
 * @return Pointer to the created device. NULL, if the type string
 *         was not specified correctly.
 *
 */
device_t *
alloc_device(char const * const type_string, char const * const device_name) {
    device_type_t const *device_type = dev_type_by_name(type_string);
    if (device_type == NULL) {
        error("Unknown type for device %s: %s", device_name, type_string);
        return NULL;
    }

    if ((!machine_nondet) && (device_type->nondet)) {
        error("Device \"%s\" results in non-deterministic behaviour.\n"
              "This is currently disabled. Use the command-line option\n"
              "-n to enable non-determinism.",
                type_string);
        return NULL;
    }

    // Allocate and initialize the device.
    device_t * const dev = safe_malloc_t(device_t);

    dev->type = device_type;
    dev->name = safe_strdup(device_name);
    dev->data = NULL;
    item_init(&dev->item);

    return dev;
}

void free_device(device_t *dev)
{
    /* Clean-up only if possible and if the device was initialized. */
    if (dev->type->done && dev->data) {
        dev->type->done(dev);
    }
    safe_free(dev->name);
    safe_free(dev);
}

void add_device(device_t *dev)
{
    list_append(&device_list, &dev->item);
}

/** Test device according to the given filter condition.
 *
 * @param device Device to be tested.
 * @param filter Condition for filtering.
 *
 * @return True if the given device matches to the filter.
 *
 */
static bool dev_match_to_filter(device_t *device, device_filter_t filter)
{
    // TODO: Add RV support
    // TODO: Add SH2E support

    ASSERT(device != NULL);

    switch (filter) {
    case DEVICE_FILTER_ALL:
        return true;
    case DEVICE_FILTER_STEP:
        return device->type->step != NULL;
    case DEVICE_FILTER_STEP4K:
        return device->type->step4k != NULL;
    case DEVICE_FILTER_MEMORY:
        return (strcmp(device->type->name, "rom") == 0) || (strcmp(device->type->name, "rwm") == 0);
    case DEVICE_FILTER_R4K_PROCESSOR:
        return (strcmp(device->type->name, "dr4kcpu") == 0);
    default:
        die(ERR_INTERN, "Unexpected device filter");
    }

    return false;
}

/** Iterate over the devices specified by the given filter
 *
 * @param device Address of the pointer to the previous device. The
 *               found device is returned throught this parameter.
 *               If NULL, then there has been no previous device,
 *               and the first device, which matches, is returned.
 *               If there is no next device, the NULL is returned.
 * @param filter Used to return only a special kind of devices.
 *
 * @return True if next device has been found.
 *
 */
bool dev_next(device_t **device, device_filter_t filter)
{
    if (*device == NULL) {
        *device = (device_t *) device_list.head;
    } else {
        *device = (device_t *) (*device)->item.next;
    }

    /* Find the first device, which matches to the filter */
    while (*device != NULL) {
        if (dev_match_to_filter(*device, filter)) {
            return true;
        }

        *device = (device_t *) (*device)->item.next;
    }

    return false;
}

/** Return the first device type starting with the specified prefix.
 *
 * Used for getting text completion in console.
 *
 * @param name_prefix  First letters of the device type to be found.
 * @param device_order Position in the device_types array from the searching
 *                     will start. Next start position is returned through
 *                     this parameter. If the array was all searched the
 *                     number of device types is returned.
 *
 * @return Name of found searched device type or NULL, if there is not any.
 *
 */
char const *
dev_type_by_partial_name(
    char const * const name_prefix, uint32_t * const next_index
) {
    ASSERT(name_prefix != NULL);

    /* Search from the specified device */
    for (unsigned i = *next_index; i < array_len(device_types); i++) {
        char const * const device_name = device_types[i]->name;

        if (prefix(name_prefix, device_name)) {
            /* Move the order to the next device */
            *next_index = i + 1;
            return device_name;
        }
    }

    *next_index = array_len(device_types);
    return NULL;
}

/** Return the first device starting with the specified prefix.
 *
 * Used for getting text completion in console.
 *
 * @param prefix_name First letters of the device name to be found.
 * @param device      Address of the pointer to the previous device. The
 *                    found device is returned throught this parameter.
 *                    If the device pointer is NULL, then there has been
 *                    no previous device, and the first device, which
 *                    matches, is returned. If there is no next device,
 *                    the NULL is returned.
 *
 * @return Name of found searched device or NULL, if there is not any.
 *
 */
const char *dev_by_partial_name(const char *prefix_name, device_t **device)
{
    ASSERT(device != NULL);
    ASSERT(prefix_name != NULL);

    while (dev_next(device, DEVICE_FILTER_ALL)) {
        if (prefix(prefix_name, (*device)->name)) {
            break;
        }
    }

    char *found_name = *device ? (*device)->name : NULL;
    return found_name;
}

/** Return the number of devices which starts by the specified prefix
 *
 * @param prefix_name       First letters of the device name to be counted.
 * @param last_found_device Pointer to the last found device is returned
 *                          throught this parameter or NULL, if there is
 *                          not any.
 *
 * @return Number of devices found.
 *
 */
size_t dev_count_by_partial_name(const char *name_prefix,
        device_t **last_found_device)
{
    ASSERT(name_prefix != NULL);
    ASSERT(last_found_device != NULL);

    size_t count = 0;
    device_t *device = NULL;

    while (dev_next(&device, DEVICE_FILTER_ALL)) {
        if (prefix(name_prefix, device->name)) {
            count++;
            *last_found_device = device;
        }
    }

    if (count == 0) {
        *last_found_device = NULL;
    }

    return count;
}

/** Find device with given name
 *
 * @param searched_name Name of searched device.
 *
 * @return Pointer to the found device or NULL, if there is not any.
 *
 */
device_t *dev_by_name(const char *searched_name)
{
    device_t *device = NULL;

    while (dev_next(&device, DEVICE_FILTER_ALL)) {
        if (!strcmp(searched_name, device->name)) {
            break;
        }
    }

    return device;
}

/** Remove a device from the machine.
 *
 * @param Device to be removed.
 *
 */
void dev_remove(device_t *device)
{
    list_remove(&device_list, &device->item);
}

/** Generic help generation
 *
 * Function is designed to be used in device command specifications.
 * It is a general help generated automatically from the device command
 * specification.
 *
 * @param parm End token for printing all the device commands and
 *             a string as a prefix of specified device command. This prefix
 *             is expected to specify just one command.
 * @param dev  Device determining, which commands will be printed.
 *
 * @return Always true.
 *
 */
bool dev_generic_help(token_t *parm, device_t *dev)
{
    cmd_print_extended_help(dev->type->cmds, parm);
    return true;
}

/** Find appropriate generator for auto completion of device commands.
 *
 * @param parm      Second part of text, which has been written for auto
 *                  completion.
 * @param dev       Device determining, which commands will be printed.
 * @param data      This is used for returning data for found generator. If no
 *                  generator is found, it remains untouched.
 *
 */
gen_t dev_find_generator(token_t **parm, const device_t *dev,
        const void **data)
{
    /* Check if the first token is a string */
    if (parm_type(*parm) != tt_str) {
        return NULL;
    }

    const char *user_text = parm_str(*parm);

    /* Look up for device command */
    const cmd_t *cmd;
    cmd_find_res_t res = cmd_find(user_text, dev->type->cmds + 1, &cmd);

    switch (res) {
    case CMP_NO_HIT:
        break;
    case CMP_HIT:
    case CMP_PARTIAL_HIT:
    case CMP_MULTIPLE_HIT:
        if (parm_last(*parm)) {
            /* Ignore the hardwired INIT command */
            *data = dev->type->cmds + 1;

            /* Set the default command generator */
            return generator_cmd;
        }

        if (res == CMP_MULTIPLE_HIT) {
            /* Input error */
            break;
        }

        /* Continue to the next generator, if possible */
        if (cmd->find_gen) {
            return cmd->find_gen(parm, cmd, data);
        }

        break;
    }

    return NULL;
}
