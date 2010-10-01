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

#include "../arch/mmap.h"
#include "../parser.h"
#include "device.h"
#include "machine.h"
#include "../fault.h"
#include "../text.h"
#include "../io/output.h"
#include "../utils.h"

/*
 * Device structure initialization
 */

static bool mem_init(parm_link_s *parm, device_s *dev);
static bool mem_info(parm_link_s *parm, device_s *dev);
static bool mem_generic(parm_link_s *parm, device_s *dev);
static bool mem_fmap(parm_link_s *parm, device_s *dev);
static bool mem_fill(parm_link_s *parm, device_s *dev);
static bool mem_load(parm_link_s *parm, device_s *dev);
static bool mem_save(parm_link_s *parm, device_s *dev);

cmd_s dmem_cmds[] = {
	{
		"init",
		(cmd_f) mem_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "memory name" NEXT
		REQ INT "memory start address" END
	},
	{
		"help",
		(cmd_f) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Usage help",
		"Usage help",
		NOCMD
	},
	{
		"info",
		(cmd_f) mem_info,
		DEFAULT,
		DEFAULT,
		"Configuration information",
		"Configuration information",
		NOCMD
	},
	{
		"generic",
		(cmd_f) mem_generic,
		DEFAULT,
		DEFAULT,
		"Generic memory type.",
		"Generic memory type.",
		REQ INT "size" END
	},
	{
		"fmap",
		(cmd_f) mem_fmap,
		DEFAULT,
		DEFAULT,
		"Map the memory into the file.",
		"Map the memory into the file.",
		REQ STR "File name" END
	},
	{
		"fill",
		(cmd_f) mem_fill,
		DEFAULT,
		DEFAULT,
		"Fill the memory with specified character",
		"Fill the memory with specified character",
		OPT VAR "value" END
	},
	{
		"load",
		(cmd_f) mem_load,
		DEFAULT,
		DEFAULT,
		"Load the file into the memory",
		"Load the file into the memory",
		REQ STR "File name" END
	},
	{
		"save",
		(cmd_f) mem_save,
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

static void mem_done(device_s *d);

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

/** Clean up the memory
 *
 */
static void mem_clean_up(mem_area_t *area)
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
static bool mem_init(parm_link_s *parm, device_s *dev)
{
	if (max_mem_areas >= MEM_AREAS) {
		mprintf("Memory areas limit exceeded (%u)\n", MEM_AREAS);
		return false;
	}
	
	/* Initialize */
	parm_next(&parm);
	ptr_t start = parm_next_int(&parm);
	if (!addr_word_aligned(start)) {
		mprintf("Memory address must by 4-byte aligned\n");
		return false;
	}
	
	mem_areas[max_mem_areas].index = max_mem_areas;
	mem_areas[max_mem_areas].type = MEMT_NONE;
	mem_areas[max_mem_areas].writable = (dev->type->name == id_rwm);
	
	mem_areas[max_mem_areas].start = start;
	mem_areas[max_mem_areas].size = 0;
	mem_areas[max_mem_areas].data = NULL;
	
	dev->data = &mem_areas[max_mem_areas];
	max_mem_areas++;
	
	return true;
}

/** Info command implementation
 *
 */
static bool mem_info(parm_link_s *parm, device_s *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	char *size = uint32_human_readable(area->size);
	
	mprintf("Start      Size         Type\n");
	mprintf("---------- ------------ ------\n");
	mprintf("%#10" PRIx32 " %12s %s\n",
	    area->start, size, txt_mem_type[area->type]);
	
	safe_free(size);
	
	return true;
}

/** Load command implementation
 *
 * Load the contents of the file specified to the memory block.
 *
 */
static bool mem_load(parm_link_s *parm, device_s *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	
	if (area->type != MEMT_MEM) {
		/* Illegal. */
		return false;
	}
	
	FILE *file = try_fopen(path, "rb");
	if (file == NULL) {
		mprintf("%s\n", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
		return false;
	}
	
	size_t fsize;
	if (!try_ftell(file, path, &fsize)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
		return false;
	}
	
	if (fsize == 0) {
		mprintf("Empty file\n");
		try_soft_fclose(file, path);
		return false;
	}
	
	if (fsize > area->size) {
		mprintf("File size exceeds memory area size\n");
		try_soft_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
		return false;
	}
	
	size_t rd = fread(area->data, 1, fsize, file);
	if (rd != fsize) {
		io_error(path);
		try_soft_fclose(file, path);
		mprintf("%s\n", txt_file_read_err);
		return false;
	}
	
	if (!try_fclose(file, path)) {
		mprintf("%s\n", txt_file_close_err);
		return false;
	}
	
	return true;
}

/** Fill command implementation
 *
 * Fill the memory with a specified character.
 *
 */
static bool mem_fill(parm_link_s *parm, device_s *dev)
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
			mprintf("Invalid character\n");
			return false;
		}
		
		break;
	case tt_int:
		if (parm_int(parm) > 255) {
			mprintf("Integer out of range 0..255\n");
			return false;
		}
		
		c = parm_int(parm);
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
static bool mem_fmap(parm_link_s *parm, device_s *dev)
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
		mprintf("%s\n", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
		return false;
	}
	
	size_t fsize;
	if (!try_ftell(file, path, &fsize)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
		return false;
	}
	
	if (fsize == 0) {
		mprintf("Empty file\n");
		try_soft_fclose(file, path);
		return false;
	}
	
	if ((uint64_t) area->start + (uint64_t) fsize > 0x100000000ull) {
		mprintf("Mapped file exceeds the 4 GB limit\n");
		try_soft_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_fclose(file, path);
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
		mprintf("%s\n", txt_file_map_fail);
		try_soft_fclose(file, path);
		return false;
	}
	
	/* Close file */
	if (!try_fclose(file, path)) {
		mprintf("%s\n", txt_file_close_err);
		return false;
	}
	
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
static bool mem_generic(parm_link_s *parm, device_s *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	uint32_t size = parm_int(parm);
	
	if (area->type != MEMT_NONE) {
		/* Illegal. */
		return false;
	}
	
	/* Test parameter */
	if (!addr_word_aligned(size)) {
		mprintf("Memory size must be 4-byte aligned\n");
		return false;
	}
	
	if (size == 0) {
		mprintf("Memory size is illegal\n");
		return false;
	}
	
	if ((uint64_t) area->start + (uint64_t) size > 0x100000000ull) {
		mprintf("Memory would exceed the 4 GB limit\n");
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
static bool mem_save(parm_link_s *parm, device_s *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	const char *const path = parm_str(parm);
	
	/* Do not write anything
	   if the memory is not inicialized */
	if (area->type == MEMT_NONE)
		return true;
	
	FILE *file = try_fopen(path, "wb");
	if (file == NULL) {
		mprintf("%s\n", txt_file_create_err);
		return false;
	}
	
	size_t wr = fwrite(area->data, 1, area->size, file);
	if (wr != area->size) {
		io_error(path);
		try_soft_fclose(file, path);
		mprintf("%s\n", txt_file_write_err);
		return false;
	}
	
	if (!try_fclose(file, path)) {
		mprintf("%s\n", txt_file_close_err);
		return false;
	}
	
	return true;
}

/** Dispose memory device - structures, memory blocks, unmap, etc.
 *
 */
static void mem_done(device_s *dev)
{
	mem_area_t *area = (mem_area_t *) dev->data;
	mem_clean_up(area);
	safe_free(dev->name);
	
	/* Move areas down */
	size_t i;
	for (i = area->index + 1; i < max_mem_areas; i++)
		mem_areas[i - 1] = mem_areas[i];
	
	max_mem_areas--;
}
