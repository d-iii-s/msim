/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Disk with DMA
 *
 */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "ddisk.h"

#include "../text.h"
#include "../arch/mmap.h"
#include "machine.h"
#include "dcpu.h"
#include "../fault.h"
#include "../main.h"
#include "../io/output.h"
#include "../utils.h"

/** Actions the disk is performing */
enum action_e {
	ACTION_NONE,  /**< Disk is on holidays */
	ACTION_READ,  /**< Disk is reading */
	ACTION_WRITE  /**< Disk is writting */
};

/** \{ \name Register offsets */
#define REGISTER_ADDR   0   /**< Address */
#define REGISTER_SECNO  4   /**< Sector number */
#define REGISTER_STATUS 8   /**< Status/commands */
#define REGISTER_SIZE   12  /**< Disk size in bytes */
#define REGISTER_LIMIT  16  /**< Size of register block */
/* \} */

/** \{ \name Status flags */
#define STATUS_READ  0x1  /**< Read/reading */
#define STATUS_WRITE 0x2  /**< Write/writting */
#define STATUS_INT   0x4  /**< Interrupt pending */
#define STATUS_ERROR 0x8  /**< Command error */
#define STATUS_MASK  0xf  /**< Status mask */
/* \} */

/** Disk types */
enum disk_type_e {
	DISKT_NONE,  /**< Uninitialized */
	DISKT_MEM,   /**< Memory-only disk */
	DISKT_FMAP   /**< File-mapped */
};

/*
 * Device commands
 */

static bool ddisk_init(token_t *parm, device_t *dev);
static bool ddisk_info(token_t *parm, device_t *dev);
static bool ddisk_stat(token_t *parm, device_t *dev);
static bool ddisk_generic(token_t *parm, device_t *dev);
static bool ddisk_fmap(token_t *parm, device_t *dev);
static bool ddisk_fill(token_t *parm, device_t *dev);
static bool ddisk_load(token_t *parm, device_t *dev);
static bool ddisk_save(token_t *parm, device_t *dev);

cmd_t ddisk_cmds[] = {
	{
		"init",
		(fcmd_t) ddisk_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "name/disk name" NEXT
		REQ INT "addr/register block address" NEXT
		REQ INT "intno/interrupt number within 0..6" END
	},
	{
		"help",
		(fcmd_t) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display help",
		"Display help",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(fcmd_t) ddisk_info,
		DEFAULT,
		DEFAULT,
		"Configuration information",
		"Configuration information",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) ddisk_stat,
		DEFAULT,
		DEFAULT,
		"Statistics",
		"Statistics",
		NOCMD
	},
	{
		"generic",
		(fcmd_t) ddisk_generic,
		DEFAULT,
		DEFAULT,
		"Generic memory type",
		"Generic memory type",
		REQ INT "size" END
	},
	{
		"fmap",
		(fcmd_t) ddisk_fmap,
		DEFAULT,
		DEFAULT,
		"Map the memory as the file specified",
		"Map the memory as the file specified",
		REQ STR "fname/file name" END
	},
	{
		"fill",
		(fcmd_t) ddisk_fill,
		DEFAULT,
		DEFAULT,
		"Fill the memory with specified character",
		"Fill the memory with specified character",
		OPT INT "value" END
	},
	{
		"load",
		(fcmd_t) ddisk_load,
		DEFAULT,
		DEFAULT,
		"Load the memory image from the file specified",
		"Load the memory image from the file specified",
		REQ STR "fname/file name" END
	},
	{
		"save",
		(fcmd_t) ddisk_save,
		DEFAULT,
		DEFAULT,
		"Save the memory image into the file specified",
		"Save the memory image into the file specified",
		REQ STR "fname/file name" END
	},
	LAST_CMD
};

/**< Name of the disk device as presented to the user */
const char id_ddisk[] = "ddisk";

static void ddisk_done(device_t *dev);
static void ddisk_step(device_t *dev);
static void ddisk_read(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t *val);
static void ddisk_write(cpu_t *cpu, device_t *dev, ptr_t addr, uint32_t val);

/**< Ddisk object structure */
device_type_s ddisk = {
	/* Type name and description */
	.name = id_ddisk,
	.brief = "Disk simulation",
	.full = "Implementation of a simple disk with DMA",
	
	/* Functions */
	.done = ddisk_done,
	.step = ddisk_step,
	.read = ddisk_read,
	.write = ddisk_write,
	
	/* Commands */
	ddisk_cmds
};

/** Disk instance data structure */
typedef struct {
	uint32_t *img;  /**< Disk image memory */
	
	/* Configuration */
	int intno;                   /**< Interrupt number */
	enum disk_type_e disk_type;  /**< Disk type: none, memory, file-mapped */
	uint32_t addr;               /**< Disk memory location */
	uint32_t size;               /**< Disk size */
	
	/* Registers */
	uint32_t disk_wptr;    /**< Current write pointer */
	uint32_t disk_secno;   /**< Active sector to read/write */
	uint32_t disk_status;  /**< Disk status register */
	
	/* Current action variables */
	enum action_e action;  /**< Action type */
	unsigned secno;        /**< Sector number */
	unsigned cnt;          /**< Word counter */
	bool ig;               /**< Interrupt pending flag */
	
	/* Statistics */
	uint64_t intrcount;   /**< Number of interrupts */
	uint64_t cmds_read;   /**< Number of read commands */
	uint64_t cmds_write;  /**< Number of write commands */
	uint64_t cmds_error;  /**< Number of illegal commands */
} disk_data_s;

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

/** Allocate an image memory block
 *
 * @param dd Disk instance data structure
 *
 */
static void ddisk_malloc(disk_data_s *dd)
{
	dd->img = (uint32_t *) safe_malloc(dd->size);
	memset(dd->img, 0, dd->size);
	dd->disk_type = DISKT_MEM;
}

/** Free allocated memory
 *
 * @param dd Disk instance data structure
 *
 */
static void ddisk_free(disk_data_s *dd)
{
	if (dd->disk_type == DISKT_MEM) {
		dd->size = 0;
		safe_free(dd->img);
		dd->disk_type = DISKT_NONE;
	}
}

/** Cancel action processing
 *
 * @param dd Disk instance data structure
 *
 */
static void ddisk_cancel_action(disk_data_s *dd)
{
	dd->action = ACTION_NONE;
	dd->disk_wptr = 0;
	dd->cnt = 0;
}

/** Clean up old configuration
 *
 * @param dd Disk instance data structure
 *
 */
static void ddisk_clean_up(disk_data_s *dd)
{
	/* Cancel current action */
	ddisk_cancel_action(dd);
	
	/* Do the clean up */
	switch (dd->disk_type) {
	case DISKT_NONE:
		break;
	case DISKT_MEM:
		ddisk_free(dd);
		break;
	case DISKT_FMAP:
		try_munmap(dd->img, dd->size);
		break;
	}
	dd->size = 0;
	dd->disk_type = DISKT_NONE;
}

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return true if successful
 *
 */
static bool ddisk_init(token_t *parm, device_t *dev)
{
	disk_data_s *dd;
	
	if (dev->data) {
		mprintf("Reinicialization is not allowed\n");
		return false;
	}
	
	/* Allocate structure */
	dev->data = dd = (disk_data_s *) safe_malloc_t(disk_data_s);
	
	/* Basic structure inicialization */
	parm_next(&parm);
	dd->addr = parm_next_uint(&parm);
	dd->intno = parm_next_uint(&parm);
	dd->size = 0;
	dd->disk_wptr = 0;
	dd->disk_secno = 0;
	dd->disk_status = 0;
	dd->img = (uint32_t *) MAP_FAILED;
	
	dd->action = ACTION_NONE;
	
	dd->ig = false;
	dd->intrcount = 0;
	dd->cmds_read = 0;
	dd->cmds_write = 0;
	
	dd->disk_type = DISKT_NONE;
	
	/* Checks */
	
	/* Address alignment */
	if (!addr_word_aligned(dd->addr)) {
		mprintf("Disk address must be 4-byte aligned\n");
		free(dd);
		return false;
	}
	
	/* Address limit */
	if ((uint64_t) dd->addr + (uint64_t) REGISTER_LIMIT > 0x100000000ull) {
		mprintf("Invalid address; registers would exceed the 4GB limit\n");
		return false;
	}
	
	/* Interrupt no */
	if (dd->intno > 6) {
		mprintf(txt_intnum_range);
		free(dd);
		return false;
	}
	
	return true;
}

/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true, always successful
 *
 */
static bool ddisk_info(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	const char *stype = "*";
	char *size = uint32_human_readable(dd->size);
	
	switch (dd->disk_type) {
	case DISKT_NONE:
		stype = "uninitialized";
		break;
	case DISKT_MEM:
		stype = "mem";
		break;
	case DISKT_FMAP:
		stype = "file-map";
		break;
	}
	
	mprintf("address:0x%08x intno:%d size:%s type:%s regs(mem:0x%08x "
	    "secno:%d status:0x%x ig:%d)\n",
	    (unsigned int) dd->addr, (int) dd->intno,
	    size, stype, (unsigned int) dd->disk_wptr,
	    (int) dd->disk_secno, (unsigned int) dd->disk_status,
	    (int) dd->ig);
	
	return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true, always successful
 *
 */
static bool ddisk_stat(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	
	mprintf("Interrupt count      Commands             Reads\n");
	mprintf("-------------------- -------------------- --------------------\n");
	mprintf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
	    dd->intrcount, dd->cmds_read + dd->cmds_write + dd->cmds_error,
	    dd->cmds_read);
	
	mprintf("Writes               Errors\n");
	mprintf("-------------------- --------------------\n");
	mprintf("%20" PRIu64 " %20" PRIu64 "\n",
	    dd->cmds_write, dd->cmds_error);
	
	return true;
}

/* Make the disk mapped to a memory block
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true if successful
 *
 */
static bool ddisk_generic(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	uint32_t size = parm_uint(parm);
	
	/* Size parameter check */
	if (size & 0x1ff) {
		mprintf("Illegal disk size (should be non-zero and on 512-byte aligned)");
		return false;
	}
	
	/* Clean up old configuration
	   and break the current action */
	ddisk_clean_up(dd);
	
	dd->size = size;
	ddisk_malloc(dd);
	/* Disk type already set by ddisk_malloc. */
	
	return true;
}


/** Fmap command implementation
 *
 * Map the disk to a file. The allocated memory block is disposed.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 * @return true if successful
 *
 */
static bool ddisk_fmap(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	FILE *file = try_fopen(path, "rb+");
	if (file == NULL) {
		io_error(path);
		mprintf("%s\n", txt_file_open_err);
		return false;
	}
	
	/* File size test */
	if (!try_fseek(file, 0, SEEK_END, path)) {
		mprintf("%s\n", txt_file_seek_err);
		return false;
	}
	
	size_t fsize;
	if (!try_ftell(file, path, &fsize)) {
		mprintf("%s\n", txt_file_seek_err);
		return false;
	}
	
	if (fsize == 0) {
		mprintf("Empty file\n");
		safe_fclose(file, path);
		return false;
	}
	
	/* Align the file size to the nearest
	   smalled 512 B block */
	fsize = ALIGN_DOWN(fsize, 512);
	
	/* Disk size test */
	if (fsize == 0) {
		mprintf("File is too small; at least one sector (512 B) should be present");
		safe_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path)) {
		mprintf("%s\n", txt_file_seek_err);
		return false;
	}
	
	int fd = fileno(file);
	void *ptr = mmap(0, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (ptr == MAP_FAILED) {
		io_error(path);
		mprintf("%s\n", txt_file_map_fail);
		safe_fclose(file, path);
		return false;
	}
	
	/* Close file */
	safe_fclose(file, path);
	
	/* Upgrade structures and reset the device */
	ddisk_clean_up(dd);
	dd->size = fsize;
	dd->disk_type = DISKT_FMAP;
	dd->img = (uint32_t *) ptr;
	
	return true;
}

/** Fill command implementation
 *
 * Fill the disk image with a specified character (byte).
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return true if successful
 *
 */
static bool ddisk_fill(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	unsigned char c;
	
	/* String/character */
	if (parm_type(parm) == tt_str) {
		if ((!parm_str(parm)[0]) || (parm_str(parm)[1])) {
			mprintf("Invalid string parameter; exactly one character is necessary.\n");
			return false;
		}
		
		c = parm_str(parm)[0];
	} else {
		/* Number */
		if (parm_uint(parm) > 255) {
			mprintf("Integer constant out of range 0..255\n");
			return false;
		}
		
		c = parm_uint(parm);
	}
	
	memset(dd->img, c, dd->size);
	
	return true;
}

/** Load command implementation
 *
 * Load the content of the file "filename" to the disk image.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return true if successful
 *
 */
static bool ddisk_load(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	if (dd->disk_type == DISKT_NONE) {
		/* Illegal */
		return false;
	}
	
	/* Open file */
	FILE *file = try_fopen(path, "rb");
	if (file == NULL) {
		mprintf(txt_file_open_err);
		return false;
	}
	
	/* Read the file directly */
	size_t rd = fread(dd->img, 1, dd->size, file);
	if (rd < dd->size) {
		io_error(path);
		mprintf(txt_file_read_err);
		safe_fclose(file, path);
		return false;
	}
	
	/* Close file */
	safe_fclose(file, path);
	return true;
}

/** Save command implementation
 *
 * Save the disk content to the file specified.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return true if successful
 *
 */
static bool ddisk_save(token_t *parm, device_t *dev)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	/* Do not write anything when
	   the image is not initialized */
	if (dd->disk_type == DISKT_NONE)
		return true;
	
	/* Create file */
	FILE *file = try_fopen(path, "wb");
	if (file == NULL) {
		io_error(path);
		mprintf(txt_file_create_err);
		return true;
	}
	
	/* Write data */
	size_t wr = fwrite(dd->img, 1, dd->size, file);
	if (wr < dd->size) {
		io_error(path);
		mprintf(txt_file_write_err);
		safe_fclose(file, path);
		return false;
	}
	
	/* Close file */
	safe_fclose(file, path);
	return true;
}

/** Dispose disk
 *
 * @param d Device pointer
 *
 */
static void ddisk_done(device_t *d) {
	disk_data_s *dd = (disk_data_s *) d->data;
	
	ddisk_clean_up(dd);
	
	safe_free(d->name);
	safe_free(d->data);
}

/** Read command implementation
 *
 * @param d    Ddisk device pointer
 * @param addr Address of the read operation
 * @param val  Readed (returned) value
 *
 */
static void ddisk_read(cpu_t *cpu, device_t *dev, ptr_t addr,
    uint32_t *val)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	
	/* Do nothing if the disk is not initialized. */
	if (dd->disk_type == DISKT_NONE)
		return;
	
	/* Read internal registers. */
	if (addr == dd->addr + REGISTER_ADDR)
		*val = dd->disk_wptr;
	else if (addr == dd->addr + REGISTER_SECNO)
		*val = dd->disk_secno;
	else if (addr == dd->addr + REGISTER_STATUS)
		*val = dd->disk_status;
	else if (addr == dd->addr + REGISTER_SIZE)
		*val = dd->size;
}

/** Write command implementation
 *
 * @param d    Ddisk device pointer
 * @param addr Written address
 * @param val  Value to write
 *
 */
static void ddisk_write(cpu_t *cpu, device_t *dev, ptr_t addr,
    uint32_t val)
{
	disk_data_s *dd = (disk_data_s *) dev->data;
	
	/* Ignore if the disk is not initialized */
	if (dd->disk_type == DISKT_NONE)
		return;
	
	/* Set address */
	if (addr == dd->addr + REGISTER_ADDR)
		dd->disk_wptr = val;
	else if (addr == dd->addr + REGISTER_SECNO) /* Set sector number */
		dd->disk_secno = val;
	else if (addr == dd->addr + REGISTER_STATUS) { /* Set status/command */
		/* Remove unused bits */
		dd->disk_status = val & STATUS_MASK;
		
		/* Request for interrupt deactivation */
		if (dd->disk_status & STATUS_INT) {
			dd->ig = false;
			dcpu_interrupt_down(0, dd->intno);
		}
		
		/* Check general errors */
		if ((val & STATUS_READ) && (val & STATUS_WRITE))
			return; /* Error simultaneous read/write command */
		if ((val & (STATUS_READ | STATUS_WRITE)) && (dd->action != ACTION_NONE))
			return; /* Error - command in progress */
		
		/* Read command */
		if (val & STATUS_READ) {
			/* Check bound */
			if ((dd->disk_secno + 1) * 512 -1 >= dd->size) {
				/* Error - generate interrupt */
				dd->disk_status = STATUS_INT | STATUS_ERROR;
				dcpu_interrupt_up(0, dd->intno);
				dd->ig = true;
				dd->intrcount++;
				dd->cmds_error++;
				return;
			}
			
			/* Initialize process of reading */
			dd->action = ACTION_READ;
			dd->cnt = 0;
			dd->secno = dd->disk_secno;
			dd->cmds_read++;
		}
		
		/* Write command */
		if (val & STATUS_WRITE) {
			/* Check bound */
			if ((dd->disk_secno + 1) * 512 -1 >= dd->size) {
				/* Error - generate interrupt */
				dd->disk_status = STATUS_ERROR | STATUS_INT;
				dcpu_interrupt_up(0, dd->intno);
				dd->ig = true;
				dd->intrcount++;
				dd->cmds_error++;
				return;
			}
			
			/* Initialize process of writting */
			dd->action = ACTION_WRITE;
			dd->cnt = 0;
			dd->secno = dd->disk_secno;
			dd->cmds_write++;
		}
	}
}

/** One step implementation
 *
 * @param d Ddisk device pointer
 *
 */
static void ddisk_step(device_t *d)
{
	disk_data_s *dd = (disk_data_s *) d->data;
	
	/* Reading */
	if (dd->action == ACTION_READ) {
		uint32_t val = dd->img[dd->secno * 128 + dd->cnt];
		mem_write(NULL, dd->disk_wptr, val, BITS_32, true);
		dd->disk_wptr += 4;  /* Next word */
		dd->cnt++;
		
		if (dd->cnt == 128) {
			dd->action = ACTION_NONE;
			dd->disk_status = STATUS_INT;
			dcpu_interrupt_up(0, dd->intno);
			dd->ig = true;
			dd->intrcount++;
		}
	} else if (dd->action == ACTION_WRITE) {  /* Writting */
		uint32_t val;
		val = mem_read(NULL, dd->disk_wptr, BITS_32, true);
		dd->img[dd->secno * 128 + dd->cnt] = val;
		
		dd->disk_wptr += 4;  /* Next word */
		dd->cnt++;
		
		if (dd->cnt == 128) {
			dd->action = ACTION_NONE;
			dd->disk_status = STATUS_INT;
			dcpu_interrupt_up(0, dd->intno);
			dd->ig = true;
			dd->intrcount++;
		}
	}
}
