/*
 * Disk with DMA implementation
 * Copyright (c) 2002-2004 Viliam Holub
 *
 * A disk with DMA implementation.
 */


#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "ddisk.h"

#include "mtypes.h"
#include "device.h"
#include "parser.h"
#include "machine.h"
#include "fault.h"
#include "dcpu.h"
#include "output.h"
#include "utils.h"



/*
 * Device commands
 */
static bool ddisk_init( parm_link_s *parm, device_s *dev);
static bool ddisk_info( parm_link_s *parm, device_s *dev);
static bool ddisk_stat( parm_link_s *parm, device_s *dev);
static bool ddisk_generic( parm_link_s *parm, device_s *dev);
static bool ddisk_fmap( parm_link_s *parm, device_s *dev);
static bool ddisk_fill( parm_link_s *parm, device_s *dev);
static bool ddisk_load( parm_link_s *parm, device_s *dev);
static bool ddisk_save( parm_link_s *parm, device_s *dev);

cmd_s ddisk_cmds[] =
{
	{ "init", (cmd_f)ddisk_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "name/disk name" NEXT
		REQ INT "addr/register block address" NEXT
		REQ INT "intno/interrupt number within 0..6" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Displays help",
		"Displays help",
		OPT STR "cmd/command name" END},
	{ "info", (cmd_f)ddisk_info,
		DEFAULT,
		DEFAULT,
		"Configuration informations",
		"Configuration informations",
		NOCMD},
	{ "stat", (cmd_f)ddisk_stat,
		DEFAULT,
		DEFAULT,
		"Statistics",
		"Statistics",
		NOCMD},
	{ "generic", (cmd_f)ddisk_generic,
		DEFAULT,
		DEFAULT,
		"Generic memory type.",
		"Generic memory type.",
		REQ INT "size" END},
	{ "fmap", (cmd_f)ddisk_fmap,
		DEFAULT,
		DEFAULT,
		"Map the memory into the file.",
		"Map the memory into the file.",
		REQ STR "fname/file name" END},
	{ "fill", (cmd_f)ddisk_fill,
		DEFAULT,
		DEFAULT,
		"Fill the memory with specified character",
		"Fill the memory with specified character",
		OPT INT "value" END},
	{ "load", (cmd_f)ddisk_load,
		DEFAULT,
		DEFAULT,
		"Load the file into the memory",
		"Load the file into the memory",
		REQ STR "fname/file name" END},
	{ "save", (cmd_f)ddisk_save,
		DEFAULT,
		DEFAULT,
		"Save the memory content into the file specified.",
		"Save the memory content into the file specified.",
		REQ STR "fname/file name" END},
	LAST_CMD
};

const char id_ddisk[] = "ddisk";

static void ddisk_done( device_s *d);
static void ddisk_step( device_s *d);
static void ddisk_read( device_s *d, uint32_t addr, uint32_t *val);
static void ddisk_write( device_s *d, uint32_t addr, uint32_t val);

device_type_s DDisk =
{
	/* type name */
	id_ddisk,

	/* brief description */
	"Disk simulation",
	
	/* full description */
	"Implementation of a simple disk with DMA.",
	
	/* functions */
	ddisk_done,	/* done */
	ddisk_step,	/* step */
	ddisk_read,	/* read */
	ddisk_write,	/* write */

	/* commands */
	ddisk_cmds
};


/*
 * string constants 
 * take care for position
 */
const char *txt_ddisk[] =
{
/* 0 */
	"Disk size expected.",
	"Illegal disk size (should be non-zero and on 512B boundary)",
	"Disk image expected.",
	"Mapping error.",
	"none",
/* 5 */
	"",
	"fmap",
	"File size test fail.",
	"File enlarging error.",
	"File map fail.",
/* 10 */
	"File unmap fail.",
	"Not enough memory for image allocation.",
	"Integer expected."
};


#define DISKT_NONE	0
#define DISKT_MEM	1
#define DISKT_FMAP	2

struct disk_data_s
{
	uint32_t *img;
	
	int intno;
	int disk_type;
	
	uint32_t addr, size;
	uint32_t disk_wptr, disk_secno, disk_status;

	int action;
	int cnt;
	uint32_t secno;

	bool ig;
	uint64_t intrcount, cmds_read, cmds_write, cmds_error;
};
typedef struct disk_data_s disk_data_s;



/*
 * Called to close given file descriptor when io error occures.
 * Does not break the program, just only write error message.
 */
static void
try_soft_close( int fd, const char *filename)

{
	if (close( fd))
	{
		io_error( filename);
		error( txt_pub[ 11]);
	}
}


/* safe file descriptor close */
static bool
try_close( int fd, const char *filename)

{
	if (close( fd))
	{
		io_error( filename);
		return false;
	}

	return true;
}


/* safe opening file */
static bool
try_open( int *fd, int flags, const char *filename)

{
	*fd = open( filename, flags);
	if (*fd == -1)
	{
		io_error( filename);
		return false;
	}

	return true;
}


/* safe lseek call */
static bool
try_lseek( int fd, off_t *offset, int whence, const char *filename)

{
	*offset = lseek( fd, *offset, whence);
	if (*offset == (off_t)-1)
	{
		io_error( filename);
		try_soft_close( fd, filename);

		return false;
	}

	return true;
}


/* safe munmap call; result is not relevant */
static void
try_munmap( void *s, size_t l)

{
	if (munmap( s, l) == -1)
	{
		/* huh? */
		io_error( 0);
		error( txt_ddisk[ 10]);
	}
}


/* tries alloc memory for image */
static bool
ddisk_try_malloc( disk_data_s *dd)

{
	void *p = xmalloc( dd->size);

	memset( p, 0, dd->size);
	dd->img = p;
	return true;
}



/*
 * 
 * Commands
 */


/** Init command implementation
 */
static bool
ddisk_init( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd;
	
	if (dev->data)
	{
		dprintf( "Reinicialization is not allowed.\n");
		return false;
	}
	
	/* alloc structure */
	dev->data = dd = xmalloc( sizeof( disk_data_s));
	
	/* basic structure inicialization */
	parm_next( &parm);
	dd->addr = parm_next_int( &parm);
	dd->intno = parm_next_int( &parm);
	dd->size = parm_next_int( &parm);
	dd->disk_wptr = 0;
	dd->disk_secno = 0;
	dd->disk_status = 0;
	dd->img = MAP_FAILED;
	
	dd->action = 0;
	
	dd->ig = false;
	dd->intrcount = 0;
	dd->cmds_read = 0;
	dd->cmds_write = 0;
	
	dd->disk_type = DISKT_NONE;
	
	/* checks */
	if (dd->addr & 0x3)
	{
		dprintf( "Disk address must be 4-byte aligned.\n");
		return false;
	}
	if (dd->intno > 6)
	{
		dprintf( txt_pub[ 3]);
		return false;
	}
	if (dd->size & 0x1ff)
	{
		dprintf( txt_ddisk[ 1]);
		return false;
	}
	
	return true;
}


/** Info command implementation
 */
static bool
ddisk_info( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd = dev->data;
	char s[12];
	const char *st = "*";

	cpr_num( s, dd->size);
	switch (dd->disk_type)
	{
		case DISKT_NONE:
			st = txt_ddisk[ 4];
			break;

		case DISKT_MEM:
			st = txt_ddisk[ 5];
			break;

		case DISKT_FMAP:
			st = txt_ddisk[ 6];
			break;
	}
	
	dprintf_btag( INFO_SPC, "address:0x%08x " TBRK "intno:%d " TBRK "size:%s " TBRK
			"type:%s " TBRK "regs(mem:0x%08x " TBRK
			"secno:%d " TBRK "status:0x%x " TBRK "ig:%d)\n",
			dd->addr, dd->intno, s, st,
			dd->disk_wptr, dd->disk_secno, dd->disk_status,
			dd->ig);

	return true;
}


/** Stat command implementation.
 */
static bool
ddisk_stat( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd = dev->data;

	dprintf_btag( INFO_SPC, "intrc:%d " TBRK
			"cmds total:%lld " TBRK
			"read:%lld " TBRK "write:%lld " TBRK "error:%lld\n",
			dd->intrcount,
			dd->cmds_read +dd->cmds_write +dd->cmds_error,
			dd->cmds_read, dd->cmds_write, dd->cmds_error);

	return true;
}


/** Geneneric command implementations
 */

/* Makes the disk mapped to a memory block
 */
static bool
ddisk_generic( parm_link_s *parm, device_s *dev)
	
{
	disk_data_s *dd = dev->data;
	void *ximg;
	
	switch (dd->disk_type)
	{
		case DISKT_NONE:
			if (!ddisk_try_malloc( dd))
			{
				dprintf( txt_ddisk[ 11]);
				return false;
			}
		
		case DISKT_MEM:
			memset( dd->img, 0, dd->size);
			break;
		
		case DISKT_FMAP:
			ximg = dd->img;
			if (ddisk_try_malloc( dd))
				try_munmap( ximg, dd->size);
			else
			{
				dprintf( txt_ddisk[ 11]);
				return false;
			}
			
			break;
	}

	return true;
}


/** Fmap command implementation
 *
 * Maps the disk to a file. The allocated memory block is disposed. When the file
 * size is less than the memory block size, the disk size it is enlarged.
 */
static bool
ddisk_fmap( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd = dev->data;
	const char *const filename = parm->token.tval.s;
	int fd;
	void *mx;
	off_t offset;
	ssize_t written;
	
	/* opening the file */
	fd = open( filename, O_RDWR);
	if (fd == -1)
	{
		io_error( filename);
		dprintf( txt_pub[ 8]);
		return false;
	}

	/* seeking to the end of the file to test we need to enlarge */
	offset = 0;
	if (!try_lseek( fd, &offset, SEEK_END, filename))
	{
		dprintf( txt_ddisk[ 7]);
		return false;
	}
	
	if (offset+1 < dd->size)
	{
		/* we need to enlarge file */
		/* seeking to specified memory size */
		offset = dd->size-1;
		if (!try_lseek( fd, &offset, SEEK_SET, filename))
		{
			dprintf( txt_ddisk[ 7]);
			return false;
		}
	
		/* enlarging */
		written = write( fd, "", 1);
		if (written == -1)
		{
			io_error( filename);
			try_soft_close( fd, filename);

			dprintf( txt_ddisk[ 8]);

			return false;
		}
	}

	/* file mapping */
	mx = mmap( 0, dd->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (mx == MAP_FAILED)
	{
		io_error( filename);
		try_soft_close( fd, filename);

		dprintf( txt_ddisk[ 9]);

		return false;
	}

	/* closing file */
	if (!try_close( fd, filename))
	{
		dprintf( txt_pub[ 11]);
		return false;
	}

	/* upgrading structures and disposing previous memory block */
	switch (dd->disk_type)
	{
		case DISKT_NONE:
			break;
			
		case DISKT_MEM:
			free( dd->img);
			break;
			
		case DISKT_FMAP:
			try_munmap( dd->img, dd->size);
			break;
	}
	dd->disk_type = DISKT_FMAP;
	dd->img = mx;
	
	return true;
}


/** Fill command implementation
 *
 * Fills the disk image with a specified char.
 */
static bool
ddisk_fill( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd = dev->data;
	unsigned char c;
	
	if (parm->token.ttype == tt_str)
	{
		if (!parm->token.tval.s[ 0] || parm->token.tval.s[ 1])
		{
			dprintf( "Invalid character\n");
			return false;
		}

		c = parm->token.tval.s[ 0];
	}
	else /* tt_int */
	{
		if (parm->token.tval.i > 255)
		{
			dprintf( "integer constant out of ramge 0..255\n");
			return false;
		}

		c = parm->token.tval.i;
	}
	memset( dd->img, c, dd->size);

	return true;
}


/** Load command implementation.
 *
 * Loads the contents of the file "filename" to the disk image.
 */
static bool
ddisk_load( parm_link_s *parm, device_s *dev)

{
	disk_data_s *dd = dev->data;
	const char * const filename = parm->token.tval.s;
	int fd;
	size_t readed;
	bool b;
	
	if (dd->disk_type == DISKT_NONE)
	{
		b = ddisk_generic( parm, dev);
		if (!b)
			return b;
	}
	
	if (!try_open( &fd, O_RDONLY, filename))
	{
		dprintf( txt_pub[ 8]);
		return false;
	}
	
	readed = read( fd, dd->img, dd->size);
	if (readed == -1)
	{
		io_error( filename);
		try_soft_close( fd, filename);
		
		dprintf( txt_pub[ 10]);
		return false;
	}
	
	if (!try_close( fd, filename))
	{
		dprintf( txt_pub[ 11]);
		return false;
	}
	
	return true;
}


/** Save command implementation
 *
 * Saves the disk content to the file specified.
 */
static bool
ddisk_save( parm_link_s *parm, device_s *dev)

{
	const char *const filename = parm->token.tval.s;
	disk_data_s *dd = dev->data;
	int fd;
	ssize_t written;

	fd = creat( filename, 0666);
	if (fd == -1)
	{
		io_error( filename);
		dprintf( txt_pub[ 13]);
		return true;
	}
	
	/* do not write anything when the image is not inicialized */
	if (dd->disk_type == DISKT_NONE)
	{
		if (!try_close( fd, filename))
			dprintf( txt_pub[ 11]);

		return true;
	}
	
	written = write( fd, dd->img, dd->size);
	
	if (written == -1)
	{
		io_error( filename);
		try_soft_close( fd, filename);

		dprintf( txt_pub[ 14]);

		return false;
	}
	
	if (!try_close( fd, filename))
	{
		dprintf( txt_pub[ 11]);
		return false;
	}
	
	return true;
}



/* disposing disk */
static void
ddisk_done( device_s *d)

{
	disk_data_s *dd = d->data;
	
	switch (dd->disk_type)
	{
		case DISKT_NONE:
			break;

		case DISKT_MEM:
			free( dd->img);
			break;

		case DISKT_FMAP:
			try_munmap( dd->img, dd->size);
			break;
	}
	
	xfree( d->name);
	free( d->data);
}
	

static void
ddisk_read( device_s *d, uint32_t addr, uint32_t *val)
	
{
	disk_data_s *dd = d->data;

	if (dd->disk_type == DISKT_NONE)
		return;
	
	if (addr == dd->addr)
		*val = dd->disk_wptr;
	else
	if (addr == dd->addr +4)
		*val = dd->disk_secno;
	else
	if (addr == dd->addr +8)
		*val = dd->disk_status;
}
	

static void
ddisk_write( device_s *d, uint32_t addr, uint32_t val)
	
{
	disk_data_s *dd = d->data;
	
	if (dd->disk_type == DISKT_NONE)
		return;
	
	if (addr == dd->addr)
		dd->disk_wptr = val;
	else
	if (addr == dd->addr +4)
		dd->disk_secno = val;
	else
	if (addr == dd->addr +8)
	{
		if (dd->disk_status & 0x4)
		{
			dd->ig = false;
			dcpu_interrupt_down( 0, dd->intno);
		}
		
		dd->disk_status = val & 0xf;
		if ((val & 3) == 3)
			return; /* error - no action */
		if (((val & 3) != 0) && (dd->action != 0))
			return; /* error - command in progress */
		
		if (val & 1)
		{
			/* reading */
			if ((dd->disk_secno+1)*512 -1 >= dd->size)
			{
				/* error - generate interrupt*/
				dd->disk_status = 0xc;
				dcpu_interrupt_up( 0, dd->intno);
				dd->ig = true;
				dd->intrcount++;
				dd->cmds_error++;
				return;
			}
			dd->action = 1;
			dd->cnt = 0;
			dd->secno = dd->disk_secno;
			dd->cmds_read++;
		}
		
		if (val & 2)
		{
			/* writting */
			if ((dd->disk_secno+1)*512 -1 >= dd->size)
			{
				/* error - generate interrupt*/
				dd->disk_status = 0xc;
				dcpu_interrupt_up( 0, dd->intno);
				dd->ig = true;
				dd->intrcount++;
				dd->cmds_error++;
				return;
			}
			dd->action = 2;
			dd->cnt = 0;
			dd->secno = dd->disk_secno;
			dd->cmds_write++;
		}
	}
}


static void
ddisk_step( device_s *d)

{
	disk_data_s *dd = d->data;
	
	/* writting */
	if (dd->action == 1)
	{
		uint32_t val = dd->img[ dd->secno*128 +dd->cnt];
		mem_write( dd->disk_wptr, val, INT32);
		dd->disk_wptr += 4;
		dd->cnt++;
		
		if (dd->cnt == 128)
		{
			dd->action = 0;
			dd->disk_status = 0x4;
			dcpu_interrupt_up( 0, dd->intno);
			dd->ig = true;
			dd->intrcount++;
		}
	}
	
	/* reading */
	if (dd->action == 2)
	{
		uint32_t val;
		val = mem_read( dd->disk_wptr);
		dd->img[ dd->secno*128 +dd->cnt] = val;
		
		dd->disk_wptr += 4;
		dd->cnt++;
		
		if (dd->cnt == 128)
		{
			dd->action = 0;
			dd->disk_status = 0x4;
			dcpu_interrupt_up( 0, dd->intno);
			dd->ig = true;
			dd->intrcount++;
		}
	}
}
