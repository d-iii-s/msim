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

const char txt_file_map_fail[] = "File map fail";
const char txt_file_unmap_fail[] = "File unmap fail";

/*
 * Memory types
 */

typedef enum {
	MEMT_NONE = 0,  /**< Uninitialized */
	MEMT_MEM  = 1,  /**< Generic */
	MEMT_FMAP = 2   /**< File mapped */
} mem_type_e;

const char *txt_mem_type[] = {
	"none",
	"mem",
	"fmap"
};


/*
 * Memory device data structure
 */

typedef struct {
	uint32_t start;       /* Memory position */
	uint32_t size;        /* Memory size in bytes */
	bool writeable;       /* Write-able flag */
	mem_type_e mem_type;  /* Memory type */
	
	mem_element_s *me;    /* Memory list element */
} mem_data_s;


/** Allocate memory block and clear it
 *
 */
static void mem_alloc_block(mem_data_s *md)
{
	void *mx = safe_malloc(md->size);
	memset(mx, 0, md->size);
	md->me->mem = (unsigned char *) mx;
	md->mem_type = MEMT_MEM;
}


/** Create memory element
 *
 * Memory blocks are organized in list.
 *
 */
static void mem_struct(mem_data_s *md, bool alloc)
{
	/* Add memory element */
	mem_element_s *e = (mem_element_s *) safe_malloc_t(mem_element_s);
	
	md->me = e;
	
	/* Initialize */
	e->start = md->start;
	e->size = md->size;
	e->writ = md->writeable;
	e->mem = 0;
	
	if (alloc)
		mem_alloc_block(md);
	
	/* Link */
	mem_link(e);
}


/** Close file safely
 *
 * On error, write error message and exit.
 *
 */
static void try_soft_close(int fd, const char *filename)
{
	if (close(fd)) {
		io_error(filename);
		error(txt_file_close_err);
	}
}


/** Safe file descriptor close
 *
 */
static bool try_close(int fd, const char *filename)
{
	if (close(fd)) {
		io_error(filename);
		return false;
	}
	
	return true;
}


/** Safe file open
 *
 */
static bool try_open(int *fd, int flags, const char *filename)
{
	*fd = open(filename, flags);
	if (*fd == -1) {
		io_error(filename);
		return false;
	}
	
	return true;
}


/** Safe lseek
 *
 */
static bool try_lseek(int fd, off_t *offset, int whence, const char *filename)
{
	*offset = lseek(fd, *offset, whence);
	if (*offset == (off_t) -1) {
		io_error(filename);
		try_soft_close(fd, filename);
		
		return false;
	}
	
	return true;
}


/** Safe munmap
 *
 */
static void try_munmap(void *s, size_t l)
{
	if (munmap(s, l) == -1) {
		io_error(NULL);
		error(txt_file_unmap_fail);
	}
}


/** Clean up the memory
 *
 */
static void mem_clean_up(mem_data_s *md)
{
	switch (md->mem_type) {
	case MEMT_NONE:
		/* Nothing to do. */
		break;
	case MEMT_MEM:
		/* Free old memory block. */
		mem_unlink(md->me);
		safe_free(md->me->mem);
		safe_free(md->me);
		break;
	case MEMT_FMAP:
		mem_unlink(md->me);
		try_munmap(md->me->mem, md->size);
		safe_free(md->me);
		break;
	}
	
	md->mem_type = MEMT_NONE;
	md->size = 0;
}


/** Init command implementation
 *
 * Initialize memory structure.
 *
 */
static bool mem_init(parm_link_s *parm, device_s *dev)
{
	mem_data_s *md = safe_malloc_t(mem_data_s);
	
	/* Allocate memory structure */
	dev->data = md;
	
	/* Initialize */
	parm_next(&parm);
	md->me = NULL;
	md->mem_type = MEMT_NONE;
	md->start = parm_next_int(&parm);
	md->size = 0;
	md->writeable = (dev->type->name == id_rwm);
	
	/* Check */
	if (!addr_word_aligned(md->start)) {
		mprintf("Memory address must by 4-byte aligned\n");
		safe_free(md);
		return false;
	}
	
	return true;
}


/** Info command implementation
 *
 */
static bool mem_info(parm_link_s *parm, device_s *dev)
{
	mem_data_s *md = dev->data;
	char s[8];
	
	cpr_num(s, md->size);
	mprintf("Start      Size         Type\n");
	mprintf("---------- ------------ ------\n");
	mprintf("%#10" PRIx32" %12s %s\n",
	    md->start, s, txt_mem_type[md->mem_type]);
	
	return true;
}


/** Load command implementation
 *
 * Load the contents of the file specified to the memory block.
 *
 */
static bool mem_load(parm_link_s *parm, device_s *dev)
{
	mem_data_s *md = dev->data;
	const char *const filename = parm_str(parm);
	
	if (md->mem_type != MEMT_MEM) {
		/* Illegal. */
		return false;
	}
	
	int fd;
	if (!try_open(&fd, O_RDONLY, filename)) {
		mprintf("%s\n", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	off_t offset = 0;
	if (!try_lseek(fd, &offset, SEEK_END, filename)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_close(fd, filename);
		return false;
	}
	
	if (offset == 0) {
		mprintf("Empty file\n");
		try_soft_close(fd, filename);
		return false;
	}
	
	if (offset > (off_t) md->size) {
		mprintf("File size exceeds memory block size\n");
		try_soft_close(fd, filename);
		return false;
	}
	
	offset = 0;
	if (!try_lseek(fd, &offset, SEEK_SET, filename)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_close(fd, filename);
		return false;
	}
	
	ssize_t readed = read(fd, md->me->mem, md->size);
	if (readed == -1) {
		io_error(filename);
		try_soft_close(fd, filename);
		mprintf("%s\n", txt_file_read_err);
		return false;
	}
	
	if (!try_close(fd, filename)) {
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
	mem_data_s *md = dev->data;
	const char *s;
	char c = '\0';
	
	switch (parm_type(parm)) {
	case tt_end:
		/* default '\0' */
		break;
	case tt_str:
		s = parm_str(parm);
		c = s[0];
		if ((!c) || (s[1])) {
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
		/* unreachable */
		break;
	}
	
	memset(md->me->mem, c, md->size);
	
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
	mem_data_s *md = dev->data;
	const char *const filename = parm_str(parm);
	int fd;
	void *mx;
	
	/* Open the file */
	if (md->writeable)
		fd = open(filename, O_RDWR);
	else
		fd = open(filename, O_RDONLY);
	
	if (fd == -1) {
		io_error(filename);
		mprintf("%s\n", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	off_t offset = 0;
	if (!try_lseek(fd, &offset, SEEK_END, filename)) {
		mprintf("%s\n", txt_file_seek_err);
		try_soft_close(fd, filename);
		return false;
	}
	
	if (offset == 0) {
		mprintf("Empty file\n");
		try_soft_close(fd, filename);
		return false;
	}
	
	if ((uint64_t) md->start + (uint64_t) offset > 0x100000000ull) {
		mprintf("Mapped file exceeds the 4 GB limit\n");
		try_soft_close(fd, filename);
		return false;
	}
	
	/* File mapping */
	if (md->writeable)
		mx = mmap(0, offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	else
		mx = mmap(0, offset, PROT_READ, MAP_SHARED, fd, 0);
	
	if (mx == MAP_FAILED) {
		io_error(filename);
		try_soft_close(fd, filename);
		
		mprintf("%s\n", txt_file_map_fail);
		return false;
	}
	
	/* Close file */
	if (!try_close(fd, filename)) {
		mprintf("%s\n", txt_file_close_err);
		return false;
	}
	
	/* Upgrade structures and
	   dispose previous memory block */
	mem_clean_up(md);
	md->mem_type = MEMT_FMAP;
	md->size = offset;
	mem_struct(md, false);
	md->me->mem = mx;
	
	return true;
}


/** Generic command implementation
 *
 * Generic command makes memory device a standard memory.
 *
 */
static bool mem_generic(parm_link_s *parm, device_s *dev)
{
	mem_data_s *md = dev->data;
	uint32_t size = parm_int(parm);
	
	/* Test parameter */
	if (size & 0x3) {
		mprintf("Memory size must be 4-byte aligned\n");
		return false;
	}
	
	if (size == 0) {
		mprintf("Memory size is illegal\n");
		return false;
	}
	
	if ((uint64_t) md->start + (uint64_t) size > 0x100000000ull) {
		mprintf("Memory would exceed the 4 GB limit\n");
		return false;
	}
	
	/* Clear old configuration. */
	mem_clean_up(md);
	
	md->mem_type = MEMT_MEM;
	md->size = size;
	mem_struct(md, true);
	
	return true;
}


/** Save command implementation
 *
 * Save the content of the memory to the file specified.
 *
 */
static bool mem_save(parm_link_s *parm, device_s *dev)
{
	mem_data_s *md = dev->data;
	const char *const filename = parm_str(parm);
	int fd;
	ssize_t written;
	
	/* Do not write anything
	   if the memory is not inicialized */
	if (md->mem_type == MEMT_NONE)
		return true;
	
	fd = creat(filename, 0666);
	if (fd == -1) {
		io_error(filename);
		mprintf("%s\n", txt_file_create_err);
		return false;
	}
	
	written = write(fd, md->me->mem, md->size);
	
	if (written == -1) {
		io_error(filename);
		try_soft_close(fd, filename);
		mprintf("%s\n", txt_file_write_err);
		return false;
	}
	
	if (!try_close(fd, filename)) {
		mprintf("%s\n", txt_file_close_err);
		return false;
	}
	
	return true;
}


/** Dispose memory device - structures, memory blocks, unmap, etc.
 *
 */
static void mem_done(device_s *d)
{
	mem_data_s *md = d->data;
	
	mem_clean_up(md);
	
	safe_free(d->name);
	safe_free(d->data);
}
