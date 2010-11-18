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

#include "../parser.h"
#include "../cpu/r4000.h"

struct device;

/** Structure describing device methods.
 *
 * NULL value means "not implemented".
 *
 */
typedef struct {
	bool nondet;              /**< Device is non-deterministic. */
	const char *const name;   /**< Device type name (i82xx etc.). */
	const char *const brief;  /**< Brief decription of the device type. */
	const char *const full;   /**< Full device type description. */
	
	/** Dispose internal data. */
	void (*done)(struct device *dev);
	
	/** Called every machine cycle. */
	void (*step)(struct device *dev);
	
	/** Called every 4096th machine cycle. */
	void (*step4)(struct device *dev);
	
	/** Device memory read */
	void (*read8)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint8_t *val);
	void (*read16)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint16_t *val);
	void (*read32)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint32_t *val);
	void (*read64)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint64_t *val);
	
	/** Device memory write */
	void (*write8)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint8_t val);
	void (*write16)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint16_t val);
	void (*write32)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint32_t val);
	void (*write64)(cpu_t *cpu, struct device *dev, ptr36_t addr,
	    uint64_t val);
	
	
	/**
	 * An array of commands supported by the device.
	 * The last command should be the LAST_CMS macro.
	 *
	 * @see device_cmd_struct
	 */
	const cmd_t *const cmds;
} device_type_t;

/** Structure describing a device instance.
 *
 */
typedef struct device {
	item_t item;
	
	const device_type_t *type;  /**< Pointer to the device type description. */
	char *name;                 /**< Device name given by the user. Must be unique. */
	void *data;                 /**< Device specific pointer where internal data are stored. */
} device_t;

typedef enum {
	DEVICE_FILTER_ALL,
	DEVICE_FILTER_STEP,
	DEVICE_FILTER_STEP4,
	DEVICE_FILTER_MEMORY,
	DEVICE_FILTER_PROCESSOR,
} device_filter_t;

/**
 * LAST_CMD is used in device sources to determine the last command. That's
 * only a null-command with all parameters NULL.
 */
#define LAST_CMD \
	{ NULL, NULL, NULL, 0, NULL, NULL, NULL }

extern void dev_init_framework(void);

/*
 * Functions on device structures
 */
extern device_t *alloc_device(const char *type_string,
    const char *device_name);
extern void free_device(device_t *dev);
extern void add_device(device_t *dev);

extern device_t *dev_by_name(const char *name);
extern const char *dev_type_by_partial_name(const char *prefix_name,
    uint32_t* device_order);
extern const char *dev_by_partial_name(const char *prefix_name,
    device_t **device);
extern size_t dev_count_by_partial_name(const char *prefix_name,
    device_t **device);

extern bool dev_next(device_t **dev, device_filter_t filter);

/*
 * Link/unlink device functions
 */
extern void dev_add(device_t *dev);
extern void dev_remove(device_t *dev);

/*
 * General utilities
 */
extern bool dev_generic_help(token_t *parm, device_t *dev);
extern gen_t dev_find_generator(token_t **parm, const device_t *dev,
    const void **data);

#endif
