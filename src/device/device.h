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
	
	void (*done)(device_s *d);
	void (*step)(device_s *d);
	void (*step4)(device_s *d);
	void (*read)(device_s *d, uint32_t addr, uint32_t *val);
	void (*write)(device_s *d, uint32_t addr, uint32_t val);

	const cmd_s *const cmds;
};

/*
 * LAST_CMD is used in device sources to determine the last command. That's
 * only a null-command with all NULL parameters.
 */
#define LAST_CMD {}


/*
 * Device list is an array of all device types within sources.
 */
extern const device_type_s *device_types[];

/*
 * The most frequently messages for user.
 * Device implementation should prefer these.
 */
static const char *const txt_devname_expected	= "Device name expected";
static const char *const txt_duplicate_devname	= "Duplicate device name";
static const char *const txt_devaddr_expected	= "Device address expected";
static const char *const txt_devaddr_error	= "Device address error (4b align expected)";
static const char *const txt_no_more_parms	= "No more parameters allowed";
static const char *const txt_not_en_mem		= "Not enough memory for device inicialization";
static const char *const txt_intnum_expected	= "Interrupt number expected";
static const char *const txt_intnum_range	= "Interrupt number out of range 0..6";
static const char *const txt_file_open_err	= "Could not open file";
static const char *const txt_cmd_expected	= "Command expected";
static const char *const txt_file_read_err	= "Could not read file";
static const char *const txt_file_close_err	= "Could not close file";
static const char *const txt_filename_expected	= "File name expected";
static const char *const txt_file_create_err	= "Could not create file";
static const char *const txt_file_write_err	= "Could not write to file";
static const char *const txt_unknown_cmd	= "Unknown command";
static const char *const txt_file_seek_err	= "Could not seek in the file";

/*
 * For info and stat output, please use info_printf at all times.
 * First parameter is a format string and other parameters are
 * optionally arguments as for printf().
 * INFO_SPC is a string constant useful for multi-line infos.
 */
extern void info_printf(const char *fmt, ...);
#define INFO_SPC "                      "

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
