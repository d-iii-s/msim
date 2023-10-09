/*
 * Copyright (c) 2003-2007 Viliam Holub
 * Copyright (c) 2009 Martin Decky
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Memory device
 *
 */

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include "device.h"
#include "mem.h"
#include "../arch/mmap.h"
#include "../fault.h"
#include "../parser.h"
#include "../text.h"
#include "../utils.h"
#include "../physmem.h"

/*
 * String constants
 */

const char *txt_mem_type[] = {
	"none",
	"mem",
	"fmap"
};

/** Cleanup the memory
 *
 */
static void physmem_cleanup(physmem_area_t *area)
{
	switch (area->type) {
	case MEMT_NONE:
		/* Nothing to do */
		break;
	case MEMT_MEM:
		physmem_unwire(area);
		safe_free(area->data);
		//safe_free(area->trans);
		break;
	case MEMT_FMAP:
		physmem_unwire(area);
		try_munmap(area->data, FRAMES2SIZE(area->count));
		//safe_free(area->trans);
		break;
	}

	area->type = MEMT_NONE;
	area->count = 0;
}

/** Init command implementation
 *
 * Initialize memory structure.
 *
 */
static bool mem_init(token_t *parm, device_t *dev)
{
	/* Initialize */
	parm_next(&parm);
	uint64_t _start = parm_uint_next(&parm);

	if (!phys_range(_start)) {
		error("Physical memory address out of range");
		return false;
	}

	ptr36_t start = _start;

	if (!ptr36_frame_aligned(start)) {
		error("Physical memory address must be aligned on frame boundary "
		    "(%u bytes)", FRAME_SIZE);
		return false;
	}

	// TODO: fail when areas overlap

	physmem_area_t *area = safe_malloc_t(physmem_area_t);

	area->type = MEMT_NONE;
	area->writable = (strcmp(dev->type->name, "rwm") == 0);
	area->start = ADDR2FRAME(start);
	area->count = 0;
	area->data = NULL;
	//area->trans = NULL;

	dev->data = area;

	return true;
}

/** Info command implementation
 *
 */
static bool mem_info(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	char *size = uint64_human_readable(FRAMES2SIZE(area->count));

	printf("[Start    ] [Size      ] [Type]\n"
	    "%#011" PRIx64 " %12s %s\n",
	    FRAME2ADDR(area->start), size,
	    txt_mem_type[area->type]);

	safe_free(size);

	return true;
}

/** Load command implementation
 *
 * Load the contents of the file specified to the memory block.
 *
 */
static bool mem_load(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	const char *const path = parm_str(parm);

	if (area->type == MEMT_NONE) {
		error("Physical memory area not established yet");
		return false;
	}

	if (area->type == MEMT_FMAP) {
		error("Physical memory area mapped to a file already");
		return false;
	}

	FILE *file = try_fopen(path, "rb");
	if (file == NULL)
		return false;

	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path))
		return false;

	size_t fsize;
	if (!try_ftell(file, path, &fsize))
		return false;

	if (fsize == 0) {
		error("Empty file");
		safe_fclose(file, path);
		return false;
	}

	if (fsize > FRAMES2SIZE(area->count)) {
		error("File size exceeds memory area size");
		safe_fclose(file, path);
		return false;
	}

	if (!try_fseek(file, 0, SEEK_SET, path))
		return false;

	// FIXME: invalidate binary translation
	size_t rd = fread(area->data, 1, fsize, file);
	if (rd != fsize) {
		io_error(path);
		safe_fclose(file, path);
		error("%s", txt_file_read_err);
		return false;
	}

	safe_fclose(file, path);
	return true;
}

/** Fill command implementation
 *
 * Fill the memory with a specified character.
 *
 */
static bool mem_fill(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	const char *str;
	char c = 0;

	switch (parm_type(parm)) {
	case tt_end:
		/* default '\0' */
		break;
	case tt_str:
		str = parm_str(parm);
		c = str[0];

		if ((!c) || (str[1])) {
			error("Invalid character");
			return false;
		}

		break;
	case tt_uint:
		if (parm_uint(parm) > 255) {
			error("Integer out of range 0..255");
			return false;
		}

		c = parm_uint(parm);
		break;
	default:
		intr_error("Unexpected parameter type");
		return false;
	}

	// FIXME: invalidate binary translation
	memset(area->data, c, FRAMES2SIZE(area->count));
	return true;
}

/** Fmap command implementation
 *
 * Map memory to a file.
 *
 */
static bool mem_fmap(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	FILE *file;

	if (area->type != MEMT_NONE) {
		error("Physical memory area already established");
		return false;
	}

	/* Open the file */
	if (area->writable)
		file = try_fopen(path, "rb+");
	else
		file = try_fopen(path, "rb");

	if (file == NULL)
		return false;

	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path))
		return false;

	size_t fsize;
	if (!try_ftell(file, path, &fsize))
		return false;

	/* Align the size to frame boundary */
	fsize = ALIGN_UP(fsize, FRAME_SIZE);

	if (fsize == 0) {
		error("Empty file");
		safe_fclose(file, path);
		return false;
	}

	if (!phys_range(fsize)) {
		error("File size out of physical memory range");
		safe_fclose(file, path);
		return false;
	}

	len36_t size = (len36_t) fsize;

	if (size != fsize) {
		error("Incompatible host and guest address space sizes");
		safe_fclose(file, path);
		return false;
	}

	if (!phys_range(FRAME2ADDR(area->start) + size)) {
		error("File size exceeds physical memory range");
		safe_fclose(file, path);
		return false;
	}

	if (!try_fseek(file, 0, SEEK_SET, path))
		return false;

	int fd = fileno(file);
	if (fd == -1) {
		io_error(path);
		safe_fclose(file, path);
		return false;
	}

	void *ptr;

	/* File mapping */
	if (area->writable)
		ptr = mmap(0, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	else
		ptr = mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		io_error(path);
		error("%s", txt_file_map_fail);
		safe_fclose(file, path);
		return false;
	}

	/* Close file */
	safe_fclose(file, path);

	/* Update structures */
	area->type = MEMT_FMAP;
	area->count = SIZE2FRAMES(size);
	area->data = (uint8_t *) ptr;
	//area->trans = safe_malloc(sizeof(r4k_instr_fnc_t) * SIZE2INSTRS(size));
	physmem_wire(area);

	return true;
}

/** Generic command implementation
 *
 * Generic command makes memory device a standard memory.
 *
 */
static bool mem_generic(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	uint64_t _size = parm_uint(parm);

	if (area->type != MEMT_NONE) {
		error("Physical memory area already established");
		return false;
	}

	if (_size == 0) {
		error("Physical memory area size cannot be zero");
		return false;
	}

	if (!phys_range(_size)) {
		error("Physical memory area size out of physical memory range");
		return false;
	}

	len36_t size = (len36_t) _size;

	if (!phys_range(FRAME2ADDR(area->start) + size)) {
		error("Physical memory area size exceeds physical memory range");
		return false;
	}

	if (!ptr36_frame_aligned(_size)) {
		error("Physical memory area size must be aligned on frame boundary "
		    "(%u bytes)", FRAME_SIZE);
		return false;
	}

	size_t host_size = (size_t) size;

	if (host_size != size) {
		error("Incompatible host and guest address space sizes");
		return false;
	}

	area->type = MEMT_MEM;
	area->count = SIZE2FRAMES(size);
	area->data = safe_malloc(host_size);
	// Set whole memory to 0
	memset(area->data, 0, host_size);
	//area->trans = safe_malloc(sizeof(r4k_instr_fnc_t) * SIZE2INSTRS(host_size));
	physmem_wire(area);

	return true;
}

/** Save command implementation
 *
 * Save the content of the memory to the file specified.
 *
 */
static bool mem_save(token_t *parm, device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;
	const char *const path = parm_str(parm);

	if (area->type == MEMT_NONE) {
		error("Physical memory area not established");
		return false;
	}

	len36_t size = FRAMES2SIZE(area->count);
	size_t host_size = (size_t) size;

	if (host_size != size) {
		error("Incompatible host and guest address space sizes");
		return false;
	}

	FILE *file = try_fopen(path, "wb");
	if (file == NULL) {
		error("%s", txt_file_create_err);
		return false;
	}

	size_t wr = fwrite(area->data, 1, host_size, file);
	if (wr != host_size) {
		io_error(path);
		safe_fclose(file, path);
		error("%s", txt_file_write_err);
		return false;
	}

	safe_fclose(file, path);
	return true;
}

/** Dispose memory device - structures, memory blocks, unmap, etc.
 *
 */
static void mem_done(device_t *dev)
{
	physmem_area_t *area = (physmem_area_t *) dev->data;

	if (area != NULL) {
		physmem_cleanup(area);
		safe_free(area);
	}
}

cmd_t dmem_cmds[] = {
	{
		"init",
		(fcmd_t) mem_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "memory name" NEXT
		REQ INT "memory start address" END
	},
	{
		"help",
		(fcmd_t) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Usage help",
		"Usage help",
		NOCMD
	},
	{
		"info",
		(fcmd_t) mem_info,
		DEFAULT,
		DEFAULT,
		"Configuration information",
		"Configuration information",
		NOCMD
	},
	{
		"generic",
		(fcmd_t) mem_generic,
		DEFAULT,
		DEFAULT,
		"Generic memory type.",
		"Generic memory type.",
		REQ INT "size" END
	},
	{
		"fmap",
		(fcmd_t) mem_fmap,
		DEFAULT,
		DEFAULT,
		"Map the memory into the file.",
		"Map the memory into the file.",
		REQ STR "File name" END
	},
	{
		"fill",
		(fcmd_t) mem_fill,
		DEFAULT,
		DEFAULT,
		"Fill the memory with specified character",
		"Fill the memory with specified character",
		OPT VAR "value" END
	},
	{
		"load",
		(fcmd_t) mem_load,
		DEFAULT,
		DEFAULT,
		"Load the file into the memory",
		"Load the file into the memory",
		REQ STR "File name" END
	},
	{
		"save",
		(fcmd_t) mem_save,
		DEFAULT,
		DEFAULT,
		"Save the context of the memory into the file specified",
		"Save the context of the memory into the file specified",
		REQ STR "File name" END
	},
	LAST_CMD
};

device_type_t drom = {
	/* ROM is simulated deterministically */
	.nondet = false,

	/* Type name and description */
	.name = "rom",
	.brief = "Read-only memory",
	.full = "Read-only memory",

	/* Functions */
	.done = mem_done,

	/* Commands */
	.cmds = dmem_cmds
};

device_type_t drwm = {
	/* RAM is simulated deterministically */
	.nondet = false,

	/* Type name and description */
	.name = "rwm",
	.brief = "Read/write memory",
	.full = "Read/write memory",

	/* Functions */
	.done = mem_done,

	/* Commands */
	.cmds = dmem_cmds
};
