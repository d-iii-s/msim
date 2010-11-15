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
#include "mem.h"
#include "device.h"
#include "machine.h"
#include "../arch/mmap.h"
#include "../parser.h"
#include "../fault.h"
#include "../text.h"
#include "../utils.h"

/*
 * Device structure initialization
 */

static bool mem_init(token_t *parm, device_t *dev);
static bool mem_info(token_t *parm, device_t *dev);
static bool mem_generic(token_t *parm, device_t *dev);
static bool mem_fmap(token_t *parm, device_t *dev);
static bool mem_fill(token_t *parm, device_t *dev);
static bool mem_load(token_t *parm, device_t *dev);
static bool mem_save(token_t *parm, device_t *dev);

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

const char id_rom[] = "rom";
const char id_rwm[] = "rwm";

static void mem_done(device_t *d);

device_type_s drom = {
	/* Type name and description */
	.name = id_rom,
	.brief = "Read-only memory",
	.full = "Read-only memory",
	
	/* Functions */
	.done = mem_done,
	
	/* Commands */
	.cmds = dmem_cmds
};

device_type_s drwm = {
	/* Type name and description */
	.name = id_rwm,
	.brief = "Read/write memory",
	.full = "Read/write memory",
	
	/* Functions */
	.done = mem_done,
	
	/* Commands */
	.cmds = dmem_cmds
};

/*
 * String constants
 */

const char *txt_mem_type[] = {
	"none",
	"mem",
	"fmap"
};

/** Safe munmap
 *
 */
static void try_munmap(void *ptr, size_t size)
{
	if (munmap(ptr, size) == -1) {
		io_error(NULL);
		error(txt_file_unmap_fail);
	}
}

/** Cleanup the memory
 *
 */
static void mem_cleanup(mem_area_t *area)
{
	switch (area->type) {
	case MEMT_NONE:
		/* Nothing to do. */
		break;
	case MEMT_MEM:
		/* Free old memory block. */
		safe_free(area->data);
		break;
	case MEMT_FMAP:
		try_munmap(area->data, area->size);
		break;
	}
	
	area->type = MEMT_NONE;
	area->size = 0;
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
	ptr_t start = parm_next_uint(&parm);
	if (!addr_word_aligned(start)) {
		error("Memory address must by 4-byte aligned");
		return false;
	}
	
	mem_area_t *area = safe_malloc_t(mem_area_t);
	item_init(&area->item);
	
	area->type = MEMT_NONE;
	area->writable = (dev->type->name == id_rwm);
	
	area->start = start;
	area->size = 0;
	area->data = NULL;
	
	list_append(&mem_areas, &area->item);
	dev->data = area;
	
	return true;
}

/** Info command implementation
 *
 */
static bool mem_info(token_t *parm, device_t *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	char *size = uint32_human_readable(area->size);
	
	printf("[Start   ] [Size      ] [Type]\n");
	printf("%#10" PRIx32 " %12s %s\n",
	    area->start, size, txt_mem_type[area->type]);
	
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
	mem_area_t *area = (mem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	
	if (area->type != MEMT_MEM) {
		/* Illegal. */
		return false;
	}
	
	FILE *file = try_fopen(path, "rb");
	if (file == NULL) {
		error("%s", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
	size_t fsize;
	if (!try_ftell(file, path, &fsize)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
	if (fsize == 0) {
		error("Empty file");
		safe_fclose(file, path);
		return false;
	}
	
	if (fsize > area->size) {
		error("File size exceeds memory area size");
		safe_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
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
	mem_area_t *area = (mem_area_t *) dev->data;
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
		/* Unreachable */
		break;
	}
	
	memset(area->data, c, area->size);
	return true;
}

/** Fmap command implementation
 *
 * Map memory to a file. Allocated memory block is disposed. When the file
 * size is less than memory size it is enlarged.
 *
 */
static bool mem_fmap(token_t *parm, device_t *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	FILE *file;
	
	if (area->type != MEMT_NONE) {
		/* Illegal. */
		return false;
	}
	
	/* Open the file */
	if (area->writable)
		file = try_fopen(path, "rb+");
	else
		file = try_fopen(path, "rb");
	
	if (file == NULL) {
		io_error(path);
		error("%s", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
	size_t fsize;
	if (!try_ftell(file, path, &fsize)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
	if (fsize == 0) {
		error("Empty file");
		safe_fclose(file, path);
		return false;
	}
	
	if ((uint64_t) area->start + (uint64_t) fsize > 0x100000000ull) {
		error("Mapped file exceeds the 4 GB limit");
		safe_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path)) {
		error("%s", txt_file_seek_err);
		return false;
	}
	
	int fd = fileno(file);
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
	
	/* Upgrade structure */
	area->type = MEMT_FMAP;
	area->size = fsize;
	area->data = (unsigned char *) ptr;
	
	return true;
}

/** Generic command implementation
 *
 * Generic command makes memory device a standard memory.
 *
 */
static bool mem_generic(token_t *parm, device_t *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	uint32_t size = parm_uint(parm);
	
	if (area->type != MEMT_NONE) {
		/* Illegal. */
		return false;
	}
	
	/* Test parameter */
	if (!addr_word_aligned(size)) {
		error("Memory size must be 4-byte aligned");
		return false;
	}
	
	if (size == 0) {
		error("Memory size is illegal");
		return false;
	}
	
	if ((uint64_t) area->start + (uint64_t) size > 0x100000000ull) {
		error("Memory would exceed the 4 GB limit");
		return false;
	}
	
	area->type = MEMT_MEM;
	area->size = size;
	area->data = safe_malloc(size);
	
	return true;
}

/** Save command implementation
 *
 * Save the content of the memory to the file specified.
 *
 */
static bool mem_save(token_t *parm, device_t *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	
	/* Do not write anything
	   if the memory is not inicialized */
	if (area->type == MEMT_NONE)
		return true;
	
	FILE *file = try_fopen(path, "wb");
	if (file == NULL) {
		error("%s", txt_file_create_err);
		return false;
	}
	
	size_t wr = fwrite(area->data, 1, area->size, file);
	if (wr != area->size) {
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
	mem_area_t *area = (mem_area_t *) dev->data;
	
	mem_cleanup(area);
	list_remove(&mem_areas, &area->item);
	
	safe_free(area);
	safe_free(dev->name);
}
