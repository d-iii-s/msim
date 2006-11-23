/*
 * Memory device
 * Copyright (c) 2003,2004 Viliam Holub
 */

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#include "mem.h"

#include "parser.h"
#include "device.h"
#include "machine.h"
#include "fault.h"
#include "text.h"
#include "output.h"
#include "utils.h"


/*
 * Device structure initialization
 */

static bool mem_init( parm_link_s *parm, device_s *dev);
static bool mem_info( parm_link_s *parm, device_s *dev);
static bool mem_stat( parm_link_s *parm, device_s *dev);
static bool mem_generic( parm_link_s *parm, device_s *dev);
static bool mem_fmap( parm_link_s *parm, device_s *dev);
static bool mem_fill( parm_link_s *parm, device_s *dev);
static bool mem_load( parm_link_s *parm, device_s *dev);
static bool mem_save( parm_link_s *parm, device_s *dev);

cmd_s dmem_cmds[] =
{
	{ "init", (cmd_f)mem_init,
		DEFAULT,
		DEFAULT,
		"Inicialization",
		"Inicialization",
		REQ STR "memory name" NEXT
		REQ INT "memory start address" NEXT
		REQ INT "memory size" END},
	{ "help", (cmd_f)dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Usage help",
		"Usage help",
		NOCMD},
	{ "info", (cmd_f)mem_info,
		DEFAULT,
		DEFAULT,
		"Configuration informtions",
		"Configuration informtions",
		NOCMD},
	{ "stat", (cmd_f)mem_stat,
		DEFAULT,
		DEFAULT,
		"Statistics",
		"Statistics",
		NOCMD},
	{ "generic", (cmd_f)mem_generic,
		DEFAULT,
		DEFAULT,
		"Generic memory type.",
		"Generic memory type.",
		REQ INT "size" END},
	{ "fmap", (cmd_f)mem_fmap,
		DEFAULT,
		DEFAULT,
		"Map the memory into the file.",
		"Map the memory into the file.",
		REQ STR "File name" END},
	{ "fill", (cmd_f)mem_fill,
		DEFAULT,
		DEFAULT,
		"Fill the memory with specified character",
		"Fill the memory with specified character",
		OPT VAR "value" END},
	{ "load", (cmd_f)mem_load,
		DEFAULT,
		DEFAULT,
		"Load the file into the memory",
		"Load the file into the memory",
		REQ STR "File name" END},
	{ "save", (cmd_f)mem_save,
		DEFAULT,
		DEFAULT,
		"Save the context of the memory into the file specified",
		"Save the context of the memory into the file specified",
		REQ STR "File name" END},
	LAST_CMD
};


const char id_rom[] = "rom";
const char id_rwm[] = "rwm";

static void mem_done( device_s *d);


device_type_s DROM =
{
	/* type name */
	id_rom,

	/* brief description */
	"Read/write memory",
	
	/* full description */
	"xxx",
	
	/* functions */
	mem_done,	/* done */
	NULL,		/* step */
	NULL,		/* read */
	NULL,		/* write */

	/* commands */
	dmem_cmds
};

device_type_s DRWM =
{
	/* type name */
	id_rwm,

	/* brief description */
	"Read/write memory",
	
	/* full description */
	"xxx",
	
	/* functions */
	mem_done,	/* done */
	NULL,		/* step */
	NULL,		/* read */
	NULL,		/* write */

	/* commands */
	dmem_cmds
};


/*
 * string constants
 * take care for position
 */

const char *txt_mem[] =
{
/* 0 */
	"Memory starting address expected.",
	"Memory size expected.",
	"",
	"Memory position error (4b align expected).",
	"Memory size error (4b align expected).",
/* 5 */
	"Memory name expected.",
	"File size test fail",
	"File name expected.",
	"Integer expected.",
	"Memory name redefinition.",
/* 10 */
	"Illegal memory size",
	"File enlarging error",
	"Fill constant out of range (0..255).",
	"Memory exceed 4GB limit.",
	"File map fail",
/* 15 */
	"File unmap fail",
	"Unable to enlarge file to rom size"
};


const char *txt_mem_type[] =
{
	"none",
	"mem",
	"fmap"
};


#define MEMT_NONE	0
#define MEMT_MEM	1
#define MEMT_FMAP	2


/*
 * mem data structure
 */
struct mem_data_s
{
	uint32_t start;
	uint32_t size;
	bool writeable;
	int mem_type;
	
	mem_element_s *me;
	
	int par;
};
typedef struct mem_data_s mem_data_s;



/* allocs memory block and clears it */
static bool
mem_alloc_block( mem_data_s *md)

{
	void *mx = xmalloc( md->size);
	
	memset( mx, 0, md->size);
	md->me->mem = (char *)mx;
	md->mem_type = MEMT_MEM;
	
	return true;
}


static bool
mem_struct( mem_data_s *md, bool alloc)

{
	mem_element_s *e;
	
	/* adding memory */
	e = (mem_element_s *)
		xmalloc( sizeof( mem_element_s));
	
	md->me = e;
	
	/* inicializing */
	e->start = md->start;
	e->size = md->size;
	e->writ = md->writeable;
	e->mem = 0;
	
	if (alloc)
		if (!mem_alloc_block( md))
		{
			free( e);
			md->me = 0;
			mprintf( txt_mem[ 2]);

			return false;
		}
		
	
	/* linking */
	mem_link( e);

	return true;
}


/*
 * Called to close given file descriptor when io error occures.
 * Does not break program, just only write error message.
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
		error( txt_mem[ 15]);
	}
}


/** Init command implementation
 *
 * Inits memory structure.
 */

static bool
mem_init( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md;

	/* alloc memory structure */
	dev->data = md = xmalloc( sizeof( *md));

	/* initialize */
	parm_next( &parm);
	md->par = 0;
	md->me = 0;
	md->mem_type = MEMT_NONE;
	md->start = parm_next_int( &parm);
	md->size = parm_next_int( &parm);
	md->writeable = dev->type->name == id_rwm;

	/* checks */
	if (md->start & 0x3)
	{
		mprintf( "Memory address must by 4-byte aligned.\n");
		return false;
	}
	if (md->size & 0x3)
	{
		mprintf( "Memory size must be 4-byte aligned.\n");
		return false;
	}
	if (md->size == 0)
	{
		mprintf( "Memory size is illegal.\n");
		return false;
	}

	/* alloc memory */
	if ((long long)md->start +(long long)md->size > 0x100000000ull)
	{
		mprintf( "memory exceeded 4GB limit.\n");
		return false;
	}

	return true;
}


static void try_munmap( void *s, size_t l);


/** Info command implementation.
 */
static bool
mem_info( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	char s[ 8];
	
	cpr_num( s, md->size);
	mprintf_btag( INFO_SPC, "start:0x%08x " TBRK "size:%s " TBRK
			"type:%s\n", md->start, s,
			txt_mem_type[ md->mem_type]);

	return true;
}


/** Stat comman implementation.
 */
static bool
mem_stat( parm_link_s *parm, device_s *dev)

{
	mprintf_btag( INFO_SPC, "no statistics\n");
	return true;
}


/** Load command implementation
 *
 * Loads the contents of the file specified to the memory block.
 */
static bool
mem_load( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	const char *const filename = parm_str( parm);
	int fd;
	size_t readed;

	if (md->mem_type == MEMT_NONE)
		if (!mem_generic( parm, dev))
			return false;
	
	if (!try_open( &fd, O_RDONLY, filename))
		return txt_pub[ 8];
	
	readed = read( fd, md->me->mem, md->size);
	if (readed == -1)
	{
		io_error( filename);
		try_soft_close( fd, filename);
		return txt_pub[ 10];
	}
	
	if (!try_close( fd, filename))
		return txt_pub[ 11];
	
	return true;
}


/** Fill command implementation
 *
 * Fills the memory with a specified character.
 */
static bool
mem_fill( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	const char *s;
	char c = '\0';

	switch (parm_type( parm))
	{
		case tt_end:
			/* default '\0' */
			break;
		case tt_str:
			s = parm_str( parm);
			c = s[ 0];
			if (!c || s[ 1])
			{
				mprintf( "Invalid character.\n");
				return false;
			}
			break;
		case tt_int:
			if (parm_int( parm) > 255)
			{
				mprintf( "Integer out of range 0..255\n");
				return false;
			}
			c = parm_int( parm);
			break;
		default:
			/* unreachable */
			break;
	}
	
	memset( md->me->mem, c, md->size);

	return true;
}


/** Fmap command implementation
 *
 * Mapping memory to a file. Allocated memory block is disposed. When the file
 * size is less than memory size it is enlarged.
 */
static bool
mem_fmap( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	const char *const filename = parm_str( parm);
	int fd;
	void *mx;
	off_t offset;
	ssize_t written;
	
	/* opening the file */
	if (md->writeable)
		fd = open( filename, O_RDWR);
	else
		fd = open( filename, O_RDONLY);

	if (fd == -1)
	{
		io_error( filename);
		return txt_pub[ 8];
	}

	/* seeking to the end of the file to test we need to enlarge */
	offset = 0;
	if (!try_lseek( fd, &offset, SEEK_END, filename))
		return txt_mem[ 6];
	
	if (offset+1 < md->size)
	{
		/* we need to enlarge file */
		if (!md->writeable)
		{
			try_soft_close( fd, filename);
			return txt_mem[ 16];
		}
		
		/* seeking to specified memory size */
		offset = md->size-1;
		if (!try_lseek( fd, &offset, SEEK_SET, filename))
			return txt_mem[ 6];
	
		/* enlarging */
		written = write( fd, "", 1);
		if (written == -1)
		{
			io_error( filename);
			try_soft_close( fd, filename);

			return txt_mem[ 11];
		}
	}

	/* file mapping */
	if (md->writeable)
		mx = mmap( 0, md->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	else
		mx = mmap( 0, md->size, PROT_READ, MAP_SHARED, fd, 0);

	if (mx == MAP_FAILED)
	{
		io_error( filename);
		try_soft_close( fd, filename);

		return txt_mem[ 14];
	}

	/* closing file */
	if (!try_close( fd, filename))
		return txt_pub[ 11];

	/* upgrading structures and disposing previous memory block */
	switch (md->mem_type)
	{
		case MEMT_NONE:
			if (!mem_struct( md, false))
				return false;
			break;
			
		case MEMT_MEM:
			free( md->me->mem);
			break;
			
		case MEMT_FMAP:
			try_munmap( md->me->mem, md->size);
			break;
	}
	md->mem_type = MEMT_FMAP;
	md->me->mem = mx;
	
	return 0;
}


/** Generic command implementation
 *
 * Generic command makes memory device a standard memory.
 */
static bool
mem_generic( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	char *mx;
	
	switch (md->mem_type)
	{
		case MEMT_NONE:
			return mem_struct( md, true);

		case MEMT_MEM:
			memset( md->me->mem, 0, md->size);
			break;

		case MEMT_FMAP:
			mx = md->me->mem;
			if (mem_alloc_block( md))
			{
				try_munmap( mx, md->size);
			}
			else
				return txt_mem[ 2];
			
			break;
	}

	return 0;
}


/** Save command implementation
 *
 * Saves the content of the memory to the file specified.
 */
static bool
mem_save( parm_link_s *parm, device_s *dev)

{
	mem_data_s *md = dev->data;
	const char *const filename = parm_str( parm);
	int fd;
	ssize_t written;

	fd = creat( filename, 0666);
	if (fd == -1)
	{
		io_error( filename);
		return txt_pub[ 13];
	}
	
	/* do not write anything when the memory is not inicialized */
	if (md->mem_type == MEMT_NONE)
	{
		if (!try_close( fd, filename))
			return txt_pub[ 11];

		return 0;
	}
	
	written = write( fd, md->me->mem, md->size);
	
	if (written == -1)
	{
		io_error( filename);
		try_soft_close( fd, filename);
		return txt_pub[ 14];
	}
	
	if (!try_close( fd, filename))
		return txt_pub[ 11];
	
	return 0;
}

	
/* disposes memory device - structures, memory blocks, unmap, etc. */
static void
mem_done( device_s *d)

{
	mem_data_s *md = d->data;

	switch (md->mem_type)
	{
		case MEMT_NONE:
			break;
			
		case MEMT_MEM:
			mem_unlink( md->me);
			free( md->me->mem);
			free( md->me);
			break;
			
		case MEMT_FMAP:
			mem_unlink( md->me);
			try_munmap( md->me->mem, md->size);
			free( md->me);
			break;
	}
	
	XFREE( d->name);
	XFREE( d->data);
}
