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
 *
 * next		A pointer to the next instance. NULL at the end.
 */

struct device_s {
	const device_type_s *type;
	char *name;
	void *data;

	struct device_s *next;
	struct device_s *next_in_step;
	struct device_s *next_in_step4;
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
 * 		shuld test addr if it is relevat.
 * write	Called when write memory command is out of memory. Device
 * 		should test addr param.
 * cmds		An array of commands supportred by the device. Have a look
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

/*
 * LAST_CMD is used in device sources to determine the last command. That's
 * only a null-command with all NULL parameters.
 */
#define LAST_CMD { NULL, NULL, NULL, 0, NULL, NULL, NULL }


/*
 * Device list is an array of all device types within sources.
 */
extern const device_type_s *device_types[];

/*
 * Functions on device structures
 */
extern device_s *dev_by_name(const char *s);
extern const char *dev_by_partial_typename(const char *name,
    const device_type_s ***dt);
extern const char *dev_by_partial_name(const char *name, device_s **d);
extern int devs_by_partial_name(const char *name, device_s **d);
extern device_s *dev_by_name(const char *s);
extern bool dev_map(void *data, bool (*f)(void *, device_s *));
extern bool dev_next(device_s **d);
extern bool dev_next_in_step(device_s **d);
extern bool dev_next_in_step4(device_s **d);
extern void cpr_num(char *s, uint32_t i);

/*
 * Link/unlink device functions
 */
extern void dev_add(device_s *d);
extern void dev_remove(device_s *d);

/*
 * General utils
 */
extern bool dev_generic_help(parm_link_s *parm, device_s *dev);
extern void find_dev_gen(parm_link_s **pl, const device_s *d,
	gen_f *generator, const void **data);

/*
 * Often used tests
 */
extern bool addr_word_aligned(uint32_t addr);

#endif /* DEVICE_H_ */
