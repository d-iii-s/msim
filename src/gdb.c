/*
 * gdb.c
 * gdb support
 * Copyright (c) 2003 Viliam Holub
 */

#include "../config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "gdb.h"
#include "text.h"
#include "mtypes.h"
#include "mcons.h"
#include "fault.h"
#include "output.h"
#include "machine.h"
#include "parser.h"
#include "processor.h"

#include "dcpu.h"


/*
 * TODO:
 * qC - current thread id, reply: QC<16bit pid>
 * write mem - depending on processor num
 *
 */

int gdbd;

processor_s *gdb_pr;


/* 
 * write_byte
 * writes a byte c in hex form to *buf and increase the buf pointer
 */
static void
write_byte( char **buf, int c)

{
	*((*buf)++) = hexchar[ c>>4];
	*((*buf)++) = hexchar[ c&0xf];
}


/*
 * read_byte
 * reads a byte in the hex format form *buf and increases the buf pointer
 * take care for error conditions - end of a string or number corrupt
 */
static int
read_byte( char **buf)

{
	int r;
	
	r = hex2int( **buf);
	if (r == -1)
		return -1;

	(*buf)++;
	r <<= 4;
	r |= hex2int( **buf);
	if (r != -1)
		(*buf)++;

	return r;
}



static bool
gdb_separator( char c)

{
	return (c == ':') || (c == ',') || (c == ';');
}


/*
 * gdb_read_mem
 * reads length bytes from address addr for machine memory,
 * converts to hex and writes to buf
 */
static bool
gdb_read_mem( uint32_t addr, int length, char *buf)
{
	uint32_t x;

	if (length > GDB_BUFFER_SIZE*2+32)
		return false;

	if (addr & 0x3)
		return false;

	for (; length>3; length-=4, addr+=4)
	{
		x = mem_read( addr);
		
		write_byte( &buf, x&0xff);
		write_byte( &buf, (x>>8)&0xff);
		write_byte( &buf, (x>>16)&0xff);
		write_byte( &buf, (x>>24)&0xff);
	}

	if (length>1)
	{
		x = mem_read( addr);
		
		write_byte( &buf, x&0xff);
		write_byte( &buf, (x>>8)&0xff);

		length -= 2;
	}

	if (length)
	{
		x = mem_read( addr);
		
		write_byte( &buf, x&0xff);
	}
    
	return true;
}


/*
 * gdb_write_mem
 * writes length bytes to address addr from hex string in buf
 */
static bool
gdb_write_mem( uint32_t addr, int length, char *buf)
{
	uint32_t val;

	for (; length>3; length-=4, addr+=4)
	{
		val = read_byte( &buf);
		val <<= 8;
		val |= read_byte( &buf);
		val <<= 8;
		val |= read_byte( &buf);
		val <<= 8;
		val |= read_byte( &buf);

		if (val == -1)
			return false;

		val = ntohl( val);
		mem_write( addr, val, INT32);
	}

	if (length>2)
	{
		val = read_byte( &buf);
		val <<= 8;
		val |= read_byte( &buf);

		if (val == -1)
			return false;

		mem_write( addr, val, INT16);
	    
		length -= 2;
	}

	if (length)
	{
		val = read_byte( &buf);
		
		if (val == -1)
			return false;

		mem_write( addr, val, INT8);
	}
    
	return true;
}


/*
 * gdb_safe_read
 * reads one character from gdb remote descriptor
 * return true if succesfull
 */
static bool
gdb_safe_read( char *c)
	
{
	size_t readed;

	readed = read( gdbd, c, 1);
	if (readed == -1)
	{
		io_error( "gdb");
		return false;
	}

	*c &= 0x7f; /* normalize */

	return true;
}


/*
 * gdb_safe_write
 * writes one character to gdb remote descriptor
 * returns true if successful
 */
static bool
gdb_safe_write( char c)
	
{
	size_t written;

	dprintf( "%c", c);

	written = write( gdbd, &c, 1);
	if (written == -1)
	{
		io_error( "gdb");
		return false;
	}

	return true;
}


/*
 * gdb_get_message
 * reads a message from gdb and tests for correctness
 */
static bool
gdb_get_message( char *bufx)

{
	unsigned char cs, csr;
	char *buf = bufx;
	int count;
	char c;
    
	do {
		while (gdb_safe_read( &c) && (c != '$')) ;

		if (c != '$')
			return false;

		cs = 0;
		count = 0;
		while (count < GDB_BUFFER_SIZE)
		{
			if (!gdb_safe_read( buf))
				return false;
			
			if (*buf == '#') break;

			cs += *buf++;
			count++;
		}

		if (*buf != '#')
		{
			/* hmmm - buffer overflow */
			dprintf( "<buffer overflow>\n");
			return false;
		}
	
		*buf = 0;
		
		/* counting checksum */
		if (!gdb_safe_read( &c))
			return false;
		csr = hex2int( c) << 4;
		
		if (!gdb_safe_read( &c))
			return false;
		csr += hex2int( c);
	    
		
		if (cs != csr)
		{
			error( "gdb: checksum error\n");
		}

		/* sequence-id test */
		if ((cs == csr) && (bufx[ 2] == ':'))
		{
			if (!gdb_safe_write( bufx[ 0]))
				return false;
			
			if (!gdb_safe_write( bufx[ 1]))
				return false;
			
			for (buf=bufx+3; *buf; buf++,bufx++)
				*bufx = *buf;
			*bufx=0;
		}
	} while (cs != csr);

	/* sending acknowledgement */
	if (!gdb_safe_write( '+'))
		return false;

	return true;
}


/*
 * gdb_send_message
 * send a message to gdb
 * returns true if successful
 */
static bool
gdb_send_message( char *buf)
	
{
	char c;
	unsigned char cs;
    
	do {
		//XXX
		dprintf( "->");
		
		if (!gdb_safe_write( '$'))
			return false;
		
		cs = 0;
		
		while (*buf && gdb_safe_write( *buf))
		{
			cs += *buf;
			buf++;
		}
		
		if (*buf)
			return false;
		
		if (!gdb_safe_write( '#'))
			return false;
		if (!gdb_safe_write( hexchar[cs >> 4]))
			return false;
		if (!gdb_safe_write( hexchar[cs % 16]))
			return false;
		
		if (!gdb_safe_read( &c))
			return false;

		//XXX
		dprintf( "\n");
	} while (c != '+');

	return true;
}


/*
 * gdb_mem_dump
 * dumps count bytes of memory started from mem do buf in hex
 */
static char *
gdb_mem_dump( const char *mem, char *buf, int count)
	
{
	unsigned char c;

	for (; count; count--)
	{
		c = *mem++;
		*buf++ = hexchar[ c>>4];
		*buf++ = hexchar[ c&0xf];
	}      
	*buf = 0;
      
	return buf;
}


/*
 * gdb_mem_upload
 * reads count hex numbers (bytes) from buf and writes it to mem
 */
static char *
gdb_mem_upload( const char *buf, char *mem, int count)
	
{
	unsigned char c;

	for (; count; count--)
	{
		c = hex2int( *buf++) << 4;
		c += hex2int( *buf++);
		*mem++ = c;
	}

	return mem;
}


/*
 * gdb_read_hexnum
 * reads a hex number from *s
 * return true if there is any number
 */
static bool
gdb_read_hexnum( char **s, int *i)
{
	int i2;
    
	*i = 0;

	if (!hexadecimal( **s))
		return false;
    
	while (**s)
	{
		if ((i2 = hex2int( **s)) < 0)
			break;
	
		*i <<= 4;
		*i += i2;
	
		(*s)++;
	}
    
	return true;
}


/* gdb_convert_event
 * converts internal excepion events to gdb standards
 * this is not used for now
 */
static int
gdb_convert_event( int evno)
	
{
	switch (evno)
	{
		case 0: return 0;	/* no exception */
		case 1: return 7;	/* system call */
		case 2: return 11;	/* page fault */
		case 3: return 7;	/* read only exception */
		case 4: return 7;	/* bus error */
		case 5: return 11;	/* address error */
		case 6: return 16;	/* overflow */
		case 7: return 4;	/* illegal instruction */
		case 8: return 5;	/* breakpoint */
	}
	
	/* software generated*/
	return 7;
}


void
gdb_handle_event( int event)
{
	char gdb_buf[ GDB_BUFFER_SIZE];
	char *buf = gdb_buf;
	int sigval;
	uint32_t x;
	
	pr = cpu_find_no( 0);

    
	sigval = gdb_convert_event( event);

	*(buf++) = 'T';
	*(buf++) = hexchar[ sigval >> 4];
	*(buf++) = hexchar[ sigval & 0xf];

	*(buf++) = hexchar[ 37 >> 4];
	*(buf++) = hexchar[ 37 & 0xf];
	*(buf++) = ':';
	x = pr->pcreg &0x7fffffff;
	write_byte( &buf, x&0xff);
	write_byte( &buf, (x>>8)&0xff);
	write_byte( &buf, (x>>16)&0xff);
	write_byte( &buf, (x>>24)&0xff);
	*(buf++) = ';';

	*(buf++) = hexchar[ 72 >> 4];
	*(buf++) = hexchar[ 72 & 0xf];
	*(buf++) = ':';
	x = pr->regs[ 30];
	write_byte( &buf, x&0xff);
	write_byte( &buf, (x>>8)&0xff);
	write_byte( &buf, (x>>16)&0xff);
	write_byte( &buf, (x>>24)&0xff);
	*(buf++) = ';';

	*(buf++) = hexchar[ 29>> 4];
	*(buf++) = hexchar[ 29 & 0xf];
	*(buf++) = ':';
	x = pr->regs[ 29];
	write_byte( &buf, x&0xff);
	write_byte( &buf, (x>>8)&0xff);
	write_byte( &buf, (x>>16)&0xff);
	write_byte( &buf, (x>>24)&0xff);
	*(buf++) = ';';

	*buf = 0;

	gdb_send_message(gdb_buf);

	remote_gdb_step = true;
}


/*
 * gdb_read_registers
 * reads registers contents and writes it to buf in suitable format for gdb
 */
static void
gdb_read_registers( char *buf)

{
	pr = cpu_find_no( 0);
	
	buf = gdb_mem_dump( (char *)pr->regs, buf, 32*sizeof( uint32_t));
	/* ??? sr */
	buf = gdb_mem_dump( (char *)&pr->loreg, buf, sizeof( uint32_t));
	buf = gdb_mem_dump( (char *)&pr->hireg, buf, sizeof( uint32_t));
	buf = gdb_mem_dump( (char *)&cp0_badvaddr, buf, sizeof( uint32_t));
	buf = gdb_mem_dump( (char *)&pr->pcreg, buf, sizeof( uint32_t));
}


static void
gdb_write_registers( char *buf)

{
	pr = cpu_find_no( 0);
	
	buf = gdb_mem_upload( buf, (char *)pr->regs, 32*sizeof( uint32_t));
	/* ??? sr */
	buf = gdb_mem_upload( buf, (char *)&pr->loreg, sizeof( uint32_t));
	buf = gdb_mem_upload( buf, (char *)&pr->hireg, sizeof( uint32_t));
	buf = gdb_mem_upload( buf, (char *)&cp0_badvaddr, sizeof( uint32_t));
	buf = gdb_mem_upload( buf, (char *)&pr->pcreg, sizeof( uint32_t));
}


/*
 * gdb_cmd_read_mem
 * read mem command from gdb
 */
static void
gdb_cmd_read_mem( char *buf)

{
	char *bufx = buf;
	uint32_t addr, length;
				
	buf++;
				
	if ((gdb_read_hexnum( &buf, &addr)) && gdb_separator(*(buf++)) &&
			(gdb_read_hexnum( &buf, &length)))
	{
		if (!gdb_read_mem( addr, length, bufx))
			strcpy( bufx, "E03");
		/*
		else
			strcpy( bufx, "OK");
		*/
	}
	else
		strcpy( bufx, "E01");
}


/*
 * gdb_cmd_write_mem
 * write mem command  from gdb
 */
static void
gdb_cmd_write_mem( char *buf)

{
	char *bufx = buf;
	uint32_t addr, length;
				
	buf++;
				
	if ((gdb_read_hexnum( &buf, &addr)) && gdb_separator(*(buf++)) &&
			(gdb_read_hexnum( &buf, &length)) &&
			(*(buf++) == ':')) //XXX huh - it is not in read_mem
	{
		if (!gdb_write_mem( addr, length, buf))
			strcpy( bufx, "E03");
		else
			strcpy( bufx, "OK");
	}
	else
		strcpy( bufx, "E02");
}


/*
 * gdb_cmd_step
 * step or continue command from gdb
 */
static void
gdb_cmd_step( char *buf, bool cont)

{
	uint32_t addr;

	buf++;
	
	if (gdb_read_hexnum( &buf, &addr))
	{
		pr = cpu_find_no( 0);
		pr->pcreg = addr;
	}

	remote_gdb_one_step = !cont;
	remote_gdb_step = !cont;
}


static void
gdb_remote_done( bool fail, bool rq);


/*
 * gdb_session
 * this is main message loop
 */
void
gdb_session( int event)

{
	char buf[ GDB_BUFFER_SIZE];
    
	while (1)
	{
		if (remote_gdb_one_step)
		{
			remote_gdb_one_step = false;
			gdb_handle_event( 8);
		}
			
		if (!gdb_get_message( (char *)buf))
		{
			gdb_remote_done( true, false);
			return;
		}

		dprintf( "<- %s\n", buf);

		switch (buf[ 0])
		{
			case '?':
				sprintf( (char *)buf, "S%02x", event); //XXX
				break;
			
			/* more then one processor not allowed */
			case 'H':
				if ((buf[ 1] == 'c') && (buf[ 2] == '-') && (buf[ 3] == '1'))
					gdb_pr = cpu_find_no( 0);
				else
				if (buf[ 1] == 'g')
				{
				}
				else
					;
				
				strcpy( buf, "OK");
				break;
			case 'g':  /* get registers */
				gdb_read_registers( buf);
				break;
			
			case 'G':
				gdb_write_registers( &buf[ 1]);
				strcpy( buf, "OK");
				break;
			
			case 'm': 
				gdb_cmd_read_mem( buf);
				break;
			
			case 'M': 
				gdb_cmd_write_mem( buf);
				break;
			
			case 'c':
			case 's': 
				gdb_cmd_step( buf, buf[ 0] == 'c');
				return;
			
			case 'D':
				dprintf( "detach...\n");
				gdb_remote_done( false, true);
				return;

			case 'q': /* query */
				if (buf[ 1] == 'C')
				{
					buf[ 0] = hexchar[ gdb_pr->procno];
					buf[ 1] = hexchar[ gdb_pr->procno];
					buf[ 3] = 0;
				}
				else
					buf[ 0] = 0; /* not implemented */
				break;
			
			default:
				/* sending "not implemented" */
				buf[ 0] = 0;
				break;
		}

		gdb_send_message( buf);
	}
}


/*
 * gdb_remote_init
 * establish a connaction with remote gdb on selected port
 * traditional call = socket - bind - listen - accept
 * returns true if succesfull
 */
bool
gdb_remote_init( void)
{
	int yes = 1;
	struct sockaddr_in sa_gdb;
	struct sockaddr_in sa_srv;
#ifdef HAVE_SOCKLEN_T
	socklen_t addrlen = sizeof( sa_gdb);
#else
	int addrlen = sizeof( sa_gdb);
#endif

	if (R4000_cnt != 1)
	{
		dprintf( PACKAGE ": gdb: exactly one processor allowed\n");
		return false;
	}

	gdbd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gdbd < 0)
	{
		io_error( "socket");
		return false;
	}
	
	if (setsockopt( gdbd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes)))
	{
		io_error( "setsockopt");
		return false;
	}

	memset( &sa_srv, sizeof( sa_srv), 0);

	sa_srv.sin_family = AF_INET;
	sa_srv.sin_addr.s_addr = htonl( INADDR_ANY);
	sa_srv.sin_port = htons( remote_gdb_port);

	if (bind( gdbd, (struct sockaddr *)&sa_srv, sizeof( sa_srv)) < 0)
	{
		io_error( "bind");
		return false;
	}
    
	if (listen( gdbd, 1) < 0)
	{
		io_error( "listen");
		return false;
	}
    
	dprintf( "Waiting for GDB response on %d...\n", remote_gdb_port);    
	
	gdbd = accept( gdbd, (struct sockaddr *)&sa_gdb, &addrlen);
	if (gdbd < 0)
	{
		if (errno == EINTR)
			dprintf( "...interrupted.\n");
		else
			io_error( "accept");
		return false;
	}

	dprintf( "...done\n");

	gdb_pr = cpu_find_no( 0);
    
	return true;
}


/*
 * gdb_remote_done
 * cancels remote gdb
 * fail gets whether communication fails (io error)
 * rq detects remote request to close connection
 */
static void
gdb_remote_done( bool fail, bool rq)

{
	if (!fail)
	{
		if (rq)
			gdb_send_message( "OK");
		else
			gdb_send_message( "W00");
	}
	
	if (close( gdbd) == -1)
		io_error( "gdbd");
	gdbd = 0;

	remote_gdb = false;
	remote_gdb_conn = false;
}

