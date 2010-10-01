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

#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

#include "../mtypes.h"
#include "../parser.h"
#include "../cpu/processor.h"

struct device_type_s;
typedef struct device_type_s device_type_s;

/*
 * This structure describes a device instance.
 *
 * type		A pointer to the device type description.
 * 
 * name		A device name given by the user. Must be unique.
 * data		A device specific pointer where internal data are stored.
 */

struct device_s {
	item_t item;
	
	const device_type_s *type;
	char *name;
	void *data;
};
typedef struct device_s device_s;

/*
 * device_type_s
 *
 * This structure describes functions device can use.
 *
 * name		String constant - device type name (i82xx etc.)
 * desc_brief	A brief decription of the device type.
 * desc_full	A full textual device type description.
 * init		Inicialization, called once. There shoud be basic tests -
 * 		conflict detection, allocation memory etc. Returns
 * 		error string while fail.
 * done		Disposes internal data
 * step		Called every machine cycle.
 * step4	Called every 4096th machine cycle.
 * read		Called while memory read command is out of memory. Device
 * 		should test addr if it is relevant.
 * write	Called when write memory command is out of memory. Device
 * 		should test addr param.
 * cmds		An array of commands supported by the device. Have a look
 * 		at the device_cmd_struct structure for more information.
 * 		The last command should be the LAST_CMS macro.
 *
 * NULL value means "not implemented".
 */

struct device_type_s {
	const char *const name;
	const char *const brief;
	const char *const full;
	
	void (*done)(device_s *dev);
	void (*step)(device_s *dev);
	void (*step4)(device_s *dev);
	void (*read)(processor_t *pr, device_s *dev, ptr_t addr, uint32_t *val);
	void (*write)(processor_t *pr, device_s *dev, ptr_t addr, uint32_t val);

	const cmd_s *const cmds;
};

typedef enum {
	DEVICE_FILTER_ALL,
	DEVICE_FILTER_STEP,
	DEVICE_FILTER_STEP4,
	DEVICE_FILTER_MEMORY,
	DEVICE_FILTER_PROCESSOR,
} device_filter_t;

/*
 * LAST_CMD is used in device sources to determine the last command. That's
 * only a null-command with all NULL parameters.
 */
#define LAST_CMD { NULL, NULL, NULL, 0, NULL, NULL, NULL }

extern void dev_init_framework(void);

/*
 * Functions on device structures
 */
extern device_s *alloc_device(const char *type_string,
    const char *device_name);
extern device_s *dev_by_name(const char *s);
extern const char *dev_type_by_partial_name(const char *prefix_name,
    uint32_t* device_order);
extern const char *dev_by_partial_name(const char *prefix_name,
    device_s **device);
extern size_t dev_count_by_partial_name(const char *prefix_name,
    device_s **device);
extern device_s *dev_by_name(const char *s);
extern bool dev_next(device_s **device, device_filter_t filter);

/*
 * Link/unlink device functions
 */
extern void dev_add(device_s *d);
extern void dev_remove(device_s *d);

/*
 * General utils
 */
extern bool dev_generic_help(parm_link_s *parm, device_s *dev);
extern void dev_find_generator(parm_link_s **pl, const device_s *d,
    gen_f *generator, const void **data);

#endif /* DEVICE_H_ */
