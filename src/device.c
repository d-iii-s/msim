/*
 * Copyright (c) 2001-2004 Viliam Holub
 */


#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "device.h"

#include "mem.h"
#include "dcpu.h"
#include "dkeyboard.h"
#include "dorder.h"
#include "ddisk.h"
#include "dprinter.h"
#include "dtime.h"
#include "check.h"
#include "utils.h"



/* implemented pheriperal list */
const device_type_s *device_types[] =
{
	&DCPU,
	&DRWM,
	&DROM,
	&DPrinter,
	&DOrder,
	&DKeyboard,
	&DDisk,
	&DTime,
	0
};

device_s *devl = 0;



const char *txt_pub[] =
{
/* 0 */
	"Device name expected",
	"Duplicate device name",
	"Device address expected",
	"Device address error (4b align expected)",
	"No more parameters allowed",
/* 5 */
	"Not enough memory for device inicialization",
	"Interrupt number expected",
	"Interrupt number out of range 0..6",
	"Could not open a file",
	"Command expected",
/* 10 */
	"Could not read a file",
	"Could not close a file",
	"File name expected",
	"Could not create a file",
	"Could not write to a file",
/* 15 */
	"Unknown command"
};


void
info_printf( const char *fmt, ...)
	
{
        va_list ap;
        va_start( ap, fmt);
	
	if (!(!fmt || !*fmt))
	{
		vfprintf( stdout, fmt, ap);
	}
	
	va_end( ap);
}


bool
dev_next( device_s **d)

{
	*d = *d ? (*d)->next : devl;

	return !!*d;
}


/** dev_by_partial_typename - Returns the first device type starting with
 * the specified name.
 */
const char *
dev_by_partial_typename( const char *name, const device_type_s ***dt)

{
	const device_type_s **d =
		(dt && *dt) ? *dt : &device_types[ -1];

	if (!name)
		name = "";

	while (*++d && !prefix( name, (*d)->name)) ;

	if (dt)
		*dt = d;

	return *d ? (*d)->name : NULL;
}


/** dev_by_partial_name - Returns the first device starting with the specified
 * name.
 *
 */
const char *
dev_by_partial_name( const char *name, device_s **d)

{
	RQ( d != NULL);

	if (!name)
		name = "";

	while (dev_next( d))
		if (prefix( name, (*d)->name))
			break;

	return *d ? (*d)->name : NULL;
}


/** devs_by_partial_name - Returns a number of devices specified by by the
 * specified prefix.
 *
 * The function returns a device structure of the last founded device.
 */
int
devs_by_partial_name( const char *name, device_s **d)

{
	int cnt = 0;
	device_s *dx = NULL;

	RQ( name != NULL, d != NULL);

	while (dev_next( &dx))
		if (prefix( name, dx->name))
		{
			cnt++;
			*d = dx;
		}

	return cnt;
}


/** Tests for name duplicity.
 *
 */
device_s *
dev_by_name( const char *s)

{
	device_s *d = NULL;

	while (dev_next( &d))
		if (!strcmp( s, d->name))
			break;

	return d;
}


bool
dev_map( void *data, bool (*f)(void *, device_s *))

{
	device_s *d;

	while (dev_next( &d))
		if (f( data, d))
			return true;

	return false;
}


void
cpr_num( char *s, uint32_t i)

{
	if (i == 0)
	{
		*s = '0';
		*(s+1) = 0;
	}
	else
	if ((i & 0xfffff) == 0)
		sprintf( s, "%dM", i >> 20);
	else
	if ((i & 0x3ff) == 0)
		sprintf( s, "%dk", i >> 10);
	else
	if ((i % 1000) == 0)
		sprintf( s, "%dK", i / 1000);
	else
		sprintf( s, "%d", i);
}


void
dev_add( device_s *d)

{
	d->next = devl;
	devl = d;
}


void
dev_remove( device_s *d)

{
	device_s *g, *gx;

	if (d == devl)
		devl = d->next;
	else
	{
		for (g=devl; g && (g != d); g = g->next)
			gx = g;

		if (g)
			gx->next = g->next;
	}
}


/** Generic help generation.
 *
 * Function is designed to be used in device command specifications.
 * It is a general help generated automagically from the device command
 * specification.
 */
bool
dev_generic_help( parm_link_s *parm, device_s *dev)

{
	cmd_print_extended_help( parm, dev->type->cmds);
	
	return true;
}


/** Looks up to the command generator.
 */
/*
static void
find_dev_gen( parm_link_s **pl, const struct device_struct *d,
		gen_f *generator, void **data)

{
	RQ( pl != NULL, data != NULL, generator != NULL, d != NULL);
	RQ( *generator == NULL, *pl != NULL, *data == NULL);
	RQ( parm_type( *pl) != tt_end);

	if (parm_type( *pl) != tt_str)
	{
		// hmmm - theis no command specified
		// do not complete
		return;
	}
	
	// continue
	const cmd_struct cmds = ++d->cmds;

	while (

	if (dev_by_partial_typename( parm_str( *pl), &d))
	{
		if (dev_by_partial_typename( parm_str( *pl), &d))
		{
			if (parm_type( (*pl)->next) == tt_end)
			{
				*generator = generator_devtype; 
				return;
			}
			else
				// error
				return;
		}

		// fidn next -> device specific
		// test for full name
		if (parm_type( (*pl)->next) == tt_end)
		{
			*generator = generator_devtype; 
			return;
		}
		else
			// error
			return;
		
		*generator = generator_devtype; 
	}
}
*/


void
find_dev_gen( parm_link_s **pl, const device_s *d,
		gen_f *generator, const void **data)

{
	int res;
	const cmd_s *cmd;
	parm_link_s *plx;

	if (parm_type( *pl) != tt_str)
		// illegal command name
		return;
	
	// look up for device command
	switch (res = cmd_find( parm_str( *pl), d->type->cmds+1, &cmd))
	{
		case CMP_NH:
			// no such command
			return;
		case CMP_HIT:
		case CMP_MHIT:
			plx = *pl;
			parm_next( pl);
			if (parm_type( *pl) == tt_end)
			{
				// set the default command generator
				*generator = generator_cmd; 
				*data = d->type->cmds+1;
				*pl = plx;
				break;
			}
			else
			{
				if (res == CMP_MHIT)
					// input error
					break;

				// continue to the next generator, if possible
				if (cmd->find_gen)
					cmd->find_gen( pl, cmd, generator,
							data);
			}
			break;
	}
}
