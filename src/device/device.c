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

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>
#include "mem.h"
#include "dcpu.h"
#include "dkeyboard.h"
#include "dorder.h"
#include "ddisk.h"
#include "dprinter.h"
#include "dtime.h"
#include "device.h"
#include "../main.h"
#include "../assert.h"
#include "../utils.h"
#include "../fault.h"

/** Count of device types */
#define DEVICE_TYPE_COUNT  8

/* Implemented peripheral list */
const device_type_s *device_types[DEVICE_TYPE_COUNT] = {
	&dcpu,
	&drwm,
	&drom,
	&dprinter,
	&dorder,
	&dkeyboard,
	&ddisk,
	&dtime
};

/* List of all devices */
list_t device_list;

/** Initialize internal global variables. */
void dev_init_framework(void)
{
	list_init(&device_list);
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
device_t *alloc_device(const char *type_string, const char *device_name)
{
	const device_type_s *device_type = NULL;
	
	/* Search for device type */
	unsigned int i;
	for (i = 0; i < DEVICE_TYPE_COUNT; i++) {
		const device_type_s *type = device_types[i];
		if (strcmp(type_string, type->name) == 0) {
			device_type = type;
			break;
		}
	}
	
	if (device_type == NULL) {
		error("Unknown device type");
		return NULL;
	}
	
	/* Allocate a new instance */
	device_t *dev = safe_malloc_t(device_t);
	
	/* Inicialization */
	dev->type = device_type;
	dev->name = safe_strdup(device_name);
	dev->data = NULL;
	item_init(&dev->item);
	
	return dev;
}

void free_device(device_t *dev)
{
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
static bool dev_match_to_filter(device_t* device, device_filter_t filter)
{
	ASSERT(device != NULL);
	
	switch (filter) {
	case DEVICE_FILTER_ALL:
		return true;
	case DEVICE_FILTER_STEP:
		return device->type->step != NULL;
	case DEVICE_FILTER_STEP4:
		return device->type->step4 != NULL;
	case DEVICE_FILTER_MEMORY:
		return (device->type->name == id_rom) || (device->type->name == id_rwm);
	case DEVICE_FILTER_PROCESSOR:
		return device->type->name == id_dcpu;
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
		if (dev_match_to_filter(*device, filter))
			return true;
		
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
 *                     will start. Next start position is returned throught
 *                     this parameter. If the array was all searched the
 *                     DEVICE_TYPE_COUNT is returned.
 *
 * @return Name of found searched device type or NULL, if there is not any.
 *
 */
const char *dev_type_by_partial_name(const char *name_prefix,
    uint32_t* device_order)
{
	ASSERT(name_prefix != NULL);
	
	/* Search from the specified device */
	unsigned int i;
	for (i = *device_order; i < DEVICE_TYPE_COUNT; i++) {
		const char *device_name = device_types[i]->name;
		
		if (prefix(name_prefix, device_name)) {
			/* Move the order to the next device */
			*device_order = i + 1;
			return device_name;
		}
	}
	
	*device_order = DEVICE_TYPE_COUNT;
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
		if (prefix(prefix_name, (*device)->name))
			break;
	}
	
	char* found_name = *device ? (*device)->name : NULL;
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
	
	if (count == 0)
		*last_found_device = NULL;
	
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
		if (!strcmp(searched_name, device->name))
			break;
	}
	
	return device;
}

/** Add a new device to the machine.
 *
 * @param device Device to be added.
 *
 */
void dev_add(device_t *device)
{
	list_append(&device_list, &device->item);
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
	if (parm_type(*parm) != tt_str)
		return NULL;
	
	const char* user_text = parm_str(*parm);
	
	/* Look up for device command */
	const cmd_t *cmd;
	cmd_find_res_t res = cmd_find(user_text, dev->type->cmds + 1, &cmd);
	
	switch (res) {
	case CMP_NO_HIT:
	case CMP_PARTIAL_HIT:
		break;
	case CMP_HIT:
	case CMP_MULTIPLE_HIT:
		if (parm_last(*parm)) {
			/* Ignore the hardwired INIT command */
			*data = dev->type->cmds + 1;
			
			/* Set the default command generator */
			return generator_cmd;
		}
		
		if (res == CMP_MULTIPLE_HIT)
			/* Input error */
			break;
		
		/* Continue to the next generator, if possible */
		if (cmd->find_gen)
			return cmd->find_gen(parm, cmd, data);
		
		break;
	}
	
	return NULL;
}
