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
#include "../io/output.h"
#include "../main.h"
#include "../check.h"
#include "../cline.h"
#include "../utils.h"
#include "../fault.h"
#include "mem.h"
#include "dcpu.h"
#include "dkeyboard.h"
#include "dorder.h"
#include "ddisk.h"
#include "dprinter.h"
#include "dtime.h"
#include "device.h"

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
device_s *alloc_device(const char *type_string, const char *device_name)
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
		intr_error("Unknown device type");
		return NULL;
	}
	
	/* Allocate a new instance */
	device_s *device = (device_s *) safe_malloc_t(device_s);
	
	/* Inicialization */
	device->type = device_type;
	device->name = safe_strdup(device_name);
	device->data = NULL;
	item_init(&device->item);
	
	return device;
}

/** Test device according to the given filter condition.
 * 
 * @param device Device to be tested.
 * @param filter Condition for filtering.
 *
 * @return True if the given device matches to the filter.
 *
 */
static bool dev_match_to_filter(device_s* device, device_filter_t filter)
{
	PRE(device != NULL);
	
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
		die(ERR_INTERN, "Internal error at %s(%u)", __FILE__, __LINE__);
		return false;
	}
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
bool dev_next(device_s **device, device_filter_t filter)
{
	if (*device == NULL) {
		*device = (device_s *) device_list.head;
	} else {
		*device = (device_s *) (*device)->item.next;
	}
	
	/* Find the first device, which matches to the filter */
	while (*device != NULL) {
		if (dev_match_to_filter(*device, filter))
			return true;
		
		*device = (device_s *) (*device)->item.next;
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
	PRE(name_prefix != NULL);
	
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
const char *dev_by_partial_name(const char *prefix_name, device_s **device)
{
	PRE(device != NULL);
	PRE(prefix_name != NULL);
	
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
    device_s **last_found_device)
{
	PRE(name_prefix != NULL, last_found_device != NULL);
	
	size_t count = 0;
	device_s *device = NULL;
	
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
device_s *dev_by_name(const char *searched_name)
{
	device_s *device = NULL;
	
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
void dev_add(device_s *device)
{
	list_append(&device_list, &device->item);
}

/** Remove a device from the machine.
 *
 * @param Device to be removed.
 *
 */
void dev_remove(device_s *device)
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
bool dev_generic_help(parm_link_s *parm, device_s *dev)
{
	cmd_print_extended_help(parm, dev->type->cmds);
	
	return true;
}

/** Find appropriate generator for auto completion of device commands.
 *
 * @param pl        Second part of text, which has been written for auto
 *                  completion.
 * @param dev       Device determining, which commands will be printed.
 * @param generator This is used for returning found generator. If no generator
 *                  is found, it remains untouched.
 * @param data      This is used for returning data for found generator. If no
 *                  generator is found, it remains untouched.
 *
 */
void dev_find_generator(parm_link_s **pl, const device_s *d,
    gen_f *generator, const void **data)
{
	const cmd_s *cmd;
	parm_link_s *next_pl = (*pl)->next;
	
	if (parm_type(*pl) != tt_str)
		/* Illegal command name */
		return;
	
	const char* user_text = parm_str(*pl);
	
	/* Look up for device command */
	int res = cmd_find(user_text, d->type->cmds + 1, &cmd);
	
	switch (res) {
	case CMP_NO_HIT:
		/* No such command */
		return;
	case CMP_HIT:
	case CMP_MULTIPLE_HIT:
		if (parm_type(next_pl) == tt_end) {
			/* Set the default command generator */
			*generator = generator_cmd; 
			
			/* Ignore the hardwired INIT command */
			*data = d->type->cmds + 1;
			break;
		}
		
		if (res == CMP_MULTIPLE_HIT)
			/* Input error */
			break;
		
		/* Continue to the next generator, if possible */
		if (cmd->find_gen)
			cmd->find_gen(pl, cmd, generator, data);
		
		break;
	}
}
