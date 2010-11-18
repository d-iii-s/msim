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
#include "dcpu.h"
#include "ddisk.h"
#include "machine.h"
#include "../arch/mmap.h"
#include "../text.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"

/** Actions the disk is performing */
enum action_e {
	ACTION_NONE,  /**< Disk is on holidays */
	ACTION_READ,  /**< Disk is reading */
	ACTION_WRITE  /**< Disk is writting */
};

/** \{ \name Register offsets */
#define REGISTER_ADDR_LO   0   /**< Address (bits 0 .. 31) */
#define REGISTER_SECNO     4   /**< Sector number */
#define REGISTER_STATUS    8   /**< Status/commands */
#define REGISTER_SIZE_LO   12  /**< Disk size in bytes (bits 0 .. 31) */
#define REGISTER_ADDR_HI   16  /**< Address (bits 32 .. 35) */
#define REGISTER_SECNO_HI  20  /**< Reserved for future extension */
#define REGISTER_SIZE_HI   24  /**< Disk size in bytes (bits 32 .. 63) */
#define REGISTER_LIMIT     28  /**< Size of register block */
/* \} */

/** \{ \name Status flags */
#define STATUS_READ   0x01  /**< Read/reading */
#define STATUS_WRITE  0x02  /**< Write/writting */
#define STATUS_INT    0x04  /**< Interrupt pending */
#define STATUS_ERROR  0x08  /**< Command error */
#define STATUS_MASK   0x0f  /**< Status mask */
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
static void ddisk_read(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t *val);
static void ddisk_write(cpu_t *cpu, device_t *dev, ptr36_t addr, uint32_t val);

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
	unsigned int intno;          /**< Interrupt number */
	enum disk_type_e disk_type;  /**< Disk type: none, memory, file-mapped */
	ptr36_t addr;                /**< Disk memory location */
	uint64_t size;               /**< Disk size */
	
	/* Registers */
	ptr36_t disk_ptr;      /**< Current DMA pointer */
	uint32_t disk_secno;   /**< Active sector to read/write */
	uint32_t disk_status;  /**< Disk status register */
	
	/* Current action variables */
	enum action_e action;  /**< Action type */
	size_t secno;          /**< Sector number */
	size_t cnt;            /**< Word counter */
	bool ig;               /**< Interrupt pending flag */
	
	/* Statistics */
	uint64_t intrcount;   /**< Number of interrupts */
	uint64_t cmds_read;   /**< Number of read commands */
	uint64_t cmds_write;  /**< Number of write commands */
	uint64_t cmds_error;  /**< Number of illegal commands */
} disk_data_s;

/** Clean up old configuration
 *
 * @param data Disk instance data structure
 *
 */
static void ddisk_clean_up(disk_data_s *data)
{
	/* Cancel current action */
	data->action = ACTION_NONE;
	data->disk_ptr = 0;
	data->cnt = 0;
	
	/* Do the clean up */
	switch (data->disk_type) {
	case DISKT_NONE:
		break;
	case DISKT_MEM:
		safe_free(data->img);
		break;
	case DISKT_FMAP:
		try_munmap(data->img, data->size);
		break;
	}
	
	data->size = 0;
	data->disk_type = DISKT_NONE;
}

/** Init command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool ddisk_init(token_t *parm, device_t *dev)
{
	parm_next(&parm);
	uint64_t _addr = parm_uint_next(&parm);
	uint64_t _intno = parm_uint_next(&parm);
	
	if (!phys_range(_addr)) {
		error("Physical memory address out of range");
		return false;
	}
	
	if (!phys_range(_addr + (uint64_t) REGISTER_LIMIT)) {
		error("Invalid address, registers would exceed the physical "
		    "memory range");
		return false;
	}
	
	ptr36_t addr = _addr;
	
	if (!ptr36_word_aligned(addr)) {
		error("Physical memory address must by 4-byte aligned");
		return false;
	}
	
	if (_intno > 6) {
		error("%s", txt_intnum_range);
		return false;
	}
	
	/* Allocate structure */
	disk_data_s *data = safe_malloc_t(disk_data_s);
	dev->data = data;
	
	/* Basic structure inicialization */
	data->addr = addr;
	data->intno = _intno;
	data->size = 0;
	data->disk_ptr = 0;
	data->disk_secno = 0;
	data->disk_status = 0;
	data->img = (uint32_t *) MAP_FAILED;
	data->action = ACTION_NONE;
	data->secno = 0;
	data->cnt = 0;
	data->ig = false;
	data->intrcount = 0;
	data->cmds_read = 0;
	data->cmds_write = 0;
	data->disk_type = DISKT_NONE;
	
	return true;
}

/** Info command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool ddisk_info(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	const char *stype;
	char *size = uint64_human_readable(data->size);
	
	switch (data->disk_type) {
	case DISKT_NONE:
		stype = "none";
		break;
	case DISKT_MEM:
		stype = "mem";
		break;
	case DISKT_FMAP:
		stype = "fmap";
		break;
	default:
		stype = "*";
	}
	
	printf("[address  ] [int] [size      ] [type] [pointer] [sector] "
	    "[status] [ig]\n"
	    "%#011" PRIx64 " %-5u %12s %7s %#011" PRIx64 " %8u %8u %u\n",
	    data->addr, data->intno, size, stype, data->disk_ptr, data->disk_secno,
	    data->disk_status, data->ig);
	
	safe_free(size);
	return true;
}

/** Stat command implementation
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True (always successful)
 *
 */
static bool ddisk_stat(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	
	printf("[interrupts        ] [commands          ]\n");
	printf("[reads             ] [writes            ] [errors            ]\n");
	printf(" %20" PRIu64 " %20" PRIu64 "\n",
	    data->intrcount, data->cmds_read + data->cmds_write + data->cmds_error);
	printf("%20" PRIu64 "%20" PRIu64 " %20" PRIu64 "\n",
	    data->cmds_read, data->cmds_write, data->cmds_error);
	
	return true;
}

/* Make the disk mapped to a memory block
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool ddisk_generic(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	uint64_t size = parm_uint(parm);
	
	if (size == 0) {
		error("Disk size cannot be zero");
		return false;
	}
	
	if ((size & 0x1ffU) != 0) {
		error("Disk size must be 512-byte aligned");
		return false;
	}
	
	size_t host_size = (size_t) size;
	
	if (host_size != size) {
		error("Incompatible host and guest disk sizes");
		return false;
	}
	
	/* Clean up old configuration
	   and break the current action */
	ddisk_clean_up(data);
	
	data->img = (uint32_t *) safe_malloc(host_size);
	memset(data->img, 0, host_size);
	data->size = size;
	data->disk_type = DISKT_MEM;
	
	return true;
}

/** Fmap command implementation
 *
 * Map the disk to a file. The allocated memory block is disposed.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool ddisk_fmap(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	FILE *file = try_fopen(path, "rb+");
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
	
	/* Align the file size to the nearest
	   smalled 512 B block */
	fsize = ALIGN_DOWN(fsize, 512);
	
	/* Disk size test */
	if (fsize == 0) {
		error("File is too small; at least one sector (512 B) should be present");
		safe_fclose(file, path);
		return false;
	}
	
	uint64_t size = (uint64_t) fsize;
	
	if (size != fsize) {
		error("Incompatible host and guest disk sizes");
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
	
	void *ptr = mmap(0, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (ptr == MAP_FAILED) {
		io_error(path);
		error("%s", txt_file_map_fail);
		safe_fclose(file, path);
		return false;
	}
	
	/* Close file */
	safe_fclose(file, path);
	
	/* Upgrade structures and reset the device */
	ddisk_clean_up(data);
	data->size = size;
	data->disk_type = DISKT_FMAP;
	data->img = (uint32_t *) ptr;
	
	return true;
}

/** Fill command implementation
 *
 * Fill the disk image with a specified character (byte).
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool ddisk_fill(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
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
	
	memset(data->img, c, data->size);
	return true;
}

/** Load command implementation
 *
 * Load the content of the file "filename" to the disk image.
 *
 * @param parm Command-line parameters
 * @param dev  Device instance structure
 *
 * @return True if successful
 *
 */
static bool ddisk_load(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	if (data->disk_type == DISKT_NONE) {
		error("Disk not established");
		return false;
	}
	
	/* Open file */
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
	
	if (fsize > data->size) {
		error("File size exceeds disk size");
		safe_fclose(file, path);
		return false;
	}
	
	if (!try_fseek(file, 0, SEEK_SET, path))
		return false;
	
	/* Read the file directly */
	size_t rd = fread(data->img, 1, fsize, file);
	if (rd != fsize) {
		io_error(path);
		error("%s", txt_file_read_err);
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
 * @return True if successful
 *
 */
static bool ddisk_save(token_t *parm, device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	const char *const path = parm_str(parm);
	
	if (data->disk_type == DISKT_NONE) {
		error("Disk not established");
		return false;
	}
	
	size_t host_size = (size_t) data->size;
	
	if (host_size != data->size) {
		error("Incompatible host and guest disk sizes");
		return false;
	}
	
	/* Create file */
	FILE *file = try_fopen(path, "wb");
	if (file == NULL) {
		io_error(path);
		error("%s", txt_file_create_err);
		return true;
	}
	
	/* Write data */
	size_t wr = fwrite(data->img, 1, host_size, file);
	if (wr != host_size) {
		io_error(path);
		error("%s", txt_file_write_err);
		safe_fclose(file, path);
		return false;
	}
	
	/* Close file */
	safe_fclose(file, path);
	return true;
}

/** Dispose disk
 *
 * @param dev Device pointer
 *
 */
static void ddisk_done(device_t *dev) {
	disk_data_s *data = (disk_data_s *) dev->data;
	
	ddisk_clean_up(data);
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Read command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the read operation
 * @param val  Read (returned) value
 *
 */
static void ddisk_read(cpu_t *cpu, device_t *dev, ptr36_t addr,
    uint32_t *val)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	
	/* Do nothing if the disk is not initialized */
	if (data->disk_type == DISKT_NONE)
		return;
	
	/* Read internal registers */
	switch (addr - data->addr) {
	case REGISTER_ADDR_LO:
		*val = (uint32_t) (data->disk_ptr & 0xffffffffU);
		break;
	case REGISTER_ADDR_HI:
		*val = (uint32_t) (data->disk_ptr >> 32);
		break;
	case REGISTER_SECNO:
		*val = data->disk_secno;
		break;
	case REGISTER_SECNO_HI:
		*val = 0;
		break;
	case REGISTER_STATUS:
		*val = data->disk_status;
		break;
	case REGISTER_SIZE_LO:
		*val = (uint32_t) (data->size & 0xffffffffU);
		break;
	case REGISTER_SIZE_HI:
		*val = (uint32_t) (data->size >> 32);
		break;
	}
}

/** Write command implementation
 *
 * @param dev  Device pointer
 * @param addr Address of the write operation
 * @param val  Value to write
 *
 */
static void ddisk_write(cpu_t *cpu, device_t *dev, ptr36_t addr,
    uint32_t val)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	
	/* Ignore if the disk is not initialized */
	if (data->disk_type == DISKT_NONE)
		return;
	
	switch (addr - data->addr) {
	case REGISTER_ADDR_LO:
		data->disk_ptr &= ~((ptr36_t) 0xffffffffU);
		data->disk_ptr |= val;
		break;
	case REGISTER_ADDR_HI:
		data->disk_ptr &= (ptr36_t) 0xffffffffU;
		data->disk_ptr |= ((ptr36_t) val) << 32;
		break;
	case REGISTER_SECNO:
		data->disk_secno = val;
		break;
	case REGISTER_STATUS:
		/* Remove unused bits */
		data->disk_status = val & STATUS_MASK;
		
		/* Request for interrupt deactivation */
		if (data->disk_status & STATUS_INT) {
			data->ig = false;
			dcpu_interrupt_down(0, data->intno);
		}
		
		/* Simultaneous read/write command */
		if ((val & STATUS_READ) && (val & STATUS_WRITE))
			return;
		
		/* Command in progress */
		if ((val & (STATUS_READ | STATUS_WRITE)) &&
		    (data->action != ACTION_NONE))
			return;
		
		/* Check bound */
		if (((uint64_t) data->disk_secno + 1) * 512 > data->size) {
			/* Generate interrupt to indicate error */
			data->disk_status = STATUS_INT | STATUS_ERROR;
			dcpu_interrupt_up(0, data->intno);
			data->ig = true;
			data->intrcount++;
			data->cmds_error++;
			return;
		}
		
		/* Read command */
		if (val & STATUS_READ) {
			/* Reading in progress */
			data->action = ACTION_READ;
			data->cnt = 0;
			data->secno = data->disk_secno;
			data->cmds_read++;
		}
		
		/* Write command */
		if (val & STATUS_WRITE) {
			/* Writing in progress */
			data->action = ACTION_WRITE;
			data->cnt = 0;
			data->secno = data->disk_secno;
			data->cmds_write++;
		}
		
		break;
	}
}

/** One step implementation
 *
 * @param dev Device pointer
 *
 */
static void ddisk_step(device_t *dev)
{
	disk_data_s *data = (disk_data_s *) dev->data;
	size_t pos;
	
	/* Reading */
	switch (data->action) {
	case ACTION_READ:
		pos = data->secno * 128 + data->cnt;
		physmem_write(NULL, data->disk_ptr, data->img[pos], BITS_32, true);
		
		/* Next word */
		data->disk_ptr += 4;
		data->cnt++;
		break;
	case ACTION_WRITE:
		pos = data->secno * 128 + data->cnt;
		data->img[pos] = physmem_read(NULL, data->disk_ptr, BITS_32, true);
		
		/* Next word */
		data->disk_ptr += 4;
		data->cnt++;
		break;
	default:
		/* No further processing */
		return;
	}
	
	if (data->cnt == 128) {
		data->action = ACTION_NONE;
		data->disk_status = STATUS_INT;
		dcpu_interrupt_up(0, data->intno);
		data->ig = true;
		data->intrcount++;
	}
}
