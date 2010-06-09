/*
 * Copyright (c) 2003 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Remote debugging
 *
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../arch/network.h"
#include "../main.h"
#include "gdb.h"
#include "../text.h"
#include "../mtypes.h"
#include "../fault.h"
#include "../io/output.h"
#include "../device/machine.h"
#include "../parser.h"
#include "../utils.h"
#include "../cpu/processor.h"

#include "../device/dcpu.h"

/*
 * TODO:
 * qC - current thread id, reply: QC<16bit pid>
 * write mem - depending on processor num
 *
 */

typedef union {
	uint8_t uint8[4];
	uint32_t uint32;
} union32_t;

list_t debugger_breakpoints;
static int gdb_fd;

static bool gdb_separator(char c)
{
	return ((c == ':') || (c == ',') || (c == ';'));
}

/** Read length bytes from address addr for machine memory
 *
 * Converts to hex and writes to buf.
 *
 */
static bool gdb_read_mem(uint32_t addr, size_t length, char *buf)
{
	if (length > GDB_BUFFER_SIZE * 2 + 32)
		return false;
	
	/* The address must be dividable by 4 */
	if (addr & 0x3)
		return false;
	
	/*
	 * We read the memory byte by byte. This ensures
	 * that we will send the content of memory with correct
	 * endianess.
	 */
	while (length > 0) {
		sprintf(buf, "%02x", mem_read(NULL, addr, INT8));
		
		buf += 2;
		length -= 1;
		addr += 1;
	}
	
	return true;
}

/** Write length bytes to address addr from hex string in buf
 *
 */
static bool gdb_write_mem(ptr_t addr, size_t length, char *buf)
{
	while (length > 0) {
		unsigned int value;
		int matched = sscanf(buf, "%02x", &value);
		if (matched != 1)
			return false;
		
		if (!mem_write(NULL, addr, value, INT8, false))
			return false;
		
		buf += 2;
		length -= 1;
		addr++;
	}
	
	return true;
}

/** Read one character from gdb remote descriptor
 *
 * @return true if succesful
 *
 */
static bool gdb_safe_read(char *c)
{
	ssize_t rd = read(gdb_fd, c, 1);
	if (rd == -1) {
		io_error("gdb");
		return false;
	}
	
	/* Normalize */
	*c &= 0x7f;
	
	return true;
}

/** Write one character to gdb remote descriptor
 *
 * @return true if successful
 *
 */
static bool gdb_safe_write(char c)
{
	mprintf("%c", c);
	
	ssize_t written = write(gdb_fd, &c, 1);
	if (written == -1) {
		io_error("gdb");
		return false;
	}
	
	return true;
}

/** Read a message from gdb and test for correctness
 *
 */
static bool gdb_get_message(char *data)
{
	char *buf = data;
	unsigned char cs;
	unsigned char csr;
	
	do {
		char c;
		while ((gdb_safe_read(&c)) && (c != '$'));
		
		if (c != '$')
			return false;
		
		cs = 0;
		size_t count = 0;
		
		while (count < GDB_BUFFER_SIZE) {
			if (!gdb_safe_read(buf))
				return false;
			
			if (*buf == '#')
				break;
			
			cs += *buf++;
			count++;
		}
		
		if (*buf != '#') {
			/* Hmmm - buffer overflow */
			mprintf("<buffer overflow>\n");
			return false;
		}
		
		*buf = 0;
		
		/* Compute checksum */
		if (!gdb_safe_read(&c))
			return false;
		
		csr = hex2int(c) << 4;
		
		if (!gdb_safe_read(&c))
			return false;
		
		csr += hex2int(c);
		
		if (cs != csr) {
			// TODO: send minus
			error("gdb: checksum error\n");
		}
		
		/* Sequence-id test */
		if ((cs == csr) && (data[2] == ':')) {
			if (!gdb_safe_write(data[0]))
				return false;
			
			if (!gdb_safe_write(data[1]))
				return false;
			
			for (buf = data + 3; *buf; buf++, data++)
				*data = *buf;
			
			*data = 0;
		}
	} while (cs != csr);
	
	/* Send acknowledgement */
	if (!gdb_safe_write('+'))
		return false;
	
	return true;
}

/** Send a message to gdb
 *
 * @return true if successful
 *
 */
static bool gdb_send_message(char *buf)
{
	char c;
	
	do {
		mprintf("->");
		
		if (!gdb_safe_write('$'))
			return false;
		
		unsigned char cs = 0;
		
		while ((*buf) && (gdb_safe_write(*buf))) {
			cs += *buf;
			buf++;
		}
		
		if (*buf)
			return false;
		
		if (!gdb_safe_write('#'))
			return false;
		
		if (!gdb_safe_write(hexchar[cs >> 4]))
			return false;
		
		if (!gdb_safe_write(hexchar[cs % 16]))
			return false;
		
		if (!gdb_safe_read(&c))
			return false;
		
		mprintf("\n");
	} while (c != '+');
	
	return true;
}

/** Dump given count of registers into given buffer in hex
 *
 */
static char *gdb_registers_dump(const uint32_t *registers, char *buf, size_t count)
{
	char register_string[(sizeof(uint32_t) * 2) + 1] = "BADDCAFE";
	size_t i;
	
	for (i = 0; i < count; i++) {
		/*
		 * Gdb expects even the registers with the endianness
		 * of the remote target.
		 *
		 * The registers are not manipulated as raw memory, so
		 * they are stored in the memory with endianness
		 * defined by the compiler of MSIM. So we first convert
		 * it according to the endianness of the simulated
		 * program and then we send it byte by byte to the
		 * debugger.
		 */
		union32_t value;
		value.uint32 = convert_uint32_t_endian(registers[i]);
		
		sprintf(register_string, "%02x%02x%02x%02x", value.uint8[0],
		    value.uint8[1], value.uint8[2], value.uint8[3]);
		
		strcpy(buf, register_string);
		buf += strlen(register_string);
	}
	
	return buf;
}

static char *gdb_register_dump(uint32_t register_value, char *buf)
{
	return gdb_registers_dump(&register_value, buf, 1);
}

/** Read count hex numbers (bytes) from buf and write it to mem
 *
 */
static bool gdb_registers_upload(char **buf, uint32_t *registers, size_t count)
{
	size_t i;
	
	for (i = 0; i < count; i++) {
		unsigned int values[4];
		
		int matched = sscanf(*buf, "%02x%02x%02x%02x", &values[0],
		    &values[1], &values[2], &values[3]);
		
		if (matched != 4)
			return false;
		
		union32_t value;
		value.uint8[0] = values[0];
		value.uint8[1] = values[1];
		value.uint8[2] = values[2];
		value.uint8[3] = values[3];
		
		registers[i] = convert_uint32_t_endian(value.uint32);
		
		*buf += 8;
	}
	
	return true;
}


static bool gdb_register_upload(char **buf, uint32_t* register_value)
{
	return gdb_registers_upload(buf, register_value, 1);
}


/** Read a hex number from *s
 *
 * @return true if there is any number
 *
 */
static bool gdb_read_hexnum(char **s, unsigned int *i)
{
	*i = 0;
	
	if (!hexadecimal(**s))
		return false;
	
	while (**s) {
		int i2;
		
		if ((i2 = hex2int(**s)) < 0)
			break;
		
		*i <<= 4;
		*i += i2;
		
		(*s)++;
	}
	
	return true;
}

/** Convert internal exception events to gdb standards
 *
 * This is not used for now.
 *
 */
static int gdb_convert_event(gdb_events_t event)
{
	switch (event) {
	case GDB_EVENT_NO_EXCEPTION:
		return 0;
	case GDB_EVENT_SYSCALL:
		return 7;
	case GDB_EVENT_PAGE_FAULT:
		return 11;
	case GDB_EVENT_READ_ONLY_EXC:
		return 7;
	case GDB_EVENT_BUS_ERROR:
		return 7;
	case GDB_EVENT_ADDRESS_ERROR:
		return 11;
	case GDB_EVENT_OVERFLOW:
		return 16;
	case GDB_EVENT_ILLEGAL_INSTR:
		return 4;
	case GDB_EVENT_BREAKPOINT:
		return 5;
	}
	
	/* Software generated */
	return 7;
}

void gdb_handle_event(gdb_events_t event)
{
	char gdb_buf[GDB_BUFFER_SIZE];
	
	// TODO: Support more processors
	processor_t *pr = cpu_find_no(0);
	
	int sigval = gdb_convert_event(event);
	
	char pc_string[8 + 1];
	gdb_register_dump(pr->pc, pc_string);
	
	sprintf(gdb_buf, "T%02x%02x:%s;", sigval, 37, pc_string);
	
	gdb_send_message(gdb_buf);
	
	remote_gdb_listen = true;
}

/** Read registers contents and writes it to buf in suitable format for gdb
 *
 */
static void gdb_read_registers(char *buf)
{
	processor_t *pr = cpu_find_no(0);
	
	buf = gdb_registers_dump(pr->regs, buf, 32);
	buf = gdb_register_dump(pr->cp0[cp0_Status], buf);
	buf = gdb_register_dump(pr->loreg, buf);
	buf = gdb_register_dump(pr->hireg, buf);
	buf = gdb_register_dump(pr->cp0[cp0_BadVAddr], buf);
	buf = gdb_register_dump(pr->cp0[cp0_Cause], buf);
	buf = gdb_register_dump(pr->pc, buf);
}

static bool gdb_write_registers(char *buf)
{
	processor_t *pr = cpu_find_no(0);
	
	if (!gdb_registers_upload(&buf, pr->regs, 32))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->cp0[cp0_Status]))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->loreg))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->hireg))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->cp0[cp0_BadVAddr]))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->cp0[cp0_Cause]))
		return false;
	
	if (!gdb_register_upload(&buf, &pr->pc))
		return false;
	
	return true;
}


/** Read mem command from gdb
 *
 */
static void gdb_cmd_read_mem(char *buf, ptr_t addr, size_t length)
{
	/* Try to read the content of the memory */
	if (!gdb_read_mem(addr, length, buf))
		strcpy(buf, "E03");
}

/** Write mem command from gdb
 *
 */
static char* gdb_cmd_write_mem(char *buf, ptr_t addr, size_t length)
{
	/* Try to write the new content to the memory */
	if (!gdb_write_mem(addr, length, buf))
		return "E03";
	
	return "OK";
}

static void gdb_cmd_mem_operation(char *data, bool read)
{
	char *buf = data;
	ptr_t addr = 0;
	size_t length = 0;
	
	data++;
	
	/* Try to read and parse the command */
	if (!((gdb_read_hexnum(&data, &addr))
	    && gdb_separator(*(data++))
	    && (gdb_read_hexnum(&data, &length)) )) {
		char* error_str = read ? "E01" : "E02";
		strcpy(buf, error_str);
		
		return;
	}
	
	/* We need to get physical address */
	processor_t* proc = cpu_find_no(0);
	convert_addr(proc, &addr, false, false);
	
	/* Perform the memory operation */
	if (!read) {
		data++;
		
		char* answer = gdb_cmd_write_mem(data, addr, length);
		strcpy(buf, answer);
	} else
		gdb_cmd_read_mem(buf, addr, length);
}

/** Step or continue command from gdb
 *
 * @param step True for single stepping, false for continue.
 *
 */
static void gdb_cmd_step(char *buf, bool step)
{
	uint32_t addr;
	
	buf++;
	
	if (gdb_read_hexnum(&buf, &addr)) {
		processor_t *pr = cpu_find_no(0);
		pr->pc = addr;
	}
	
	remote_gdb_step = step;
	remote_gdb_listen = step;
}


static void gdb_process_query(char* buf) 
{
	char* query_content = buf + 1;
	
	/*
	 * We pretend that we have attached to some process,
	 * the gdb will then try to detach when quiting.
	 */
	if (strcmp(query_content, "Attached") == 0) {
		strcpy(buf, "1");
		return;
	}
	
	/* Thread ID does not have meaning here */
	if (strcmp(query_content, "C") == 0) {
		strcpy(buf, "-1");
		return;
	}
	
	/* There is no relocation of program loaded to the simulator */
	if (strcmp(query_content, "Offsets") == 0) {
		strcpy(buf, "Text=0;Data=0;Bss=0");
		return;
	}
	
	/* The supported features list has not been implemented yet */
	if (strcmp(query_content, "Supported") == 0) {
		strcpy(buf, "");
		return;
	}
	
	/* Symbol lookup, we do not need it -> send ok */
	if (strcmp(query_content, "Symbol::") == 0) {
		strcpy(buf, "OK");
		return;
	}
	
	/* Tracepoints are not supported */
	if (strcmp(query_content, "TStatus") == 0) {
		strcpy(buf, "tnotrun:0");
		return;
	}
	
	strcpy(buf, "E00");
}

static void gdb_insert_debugger_breakpoint(ptr_t addr)
{
	/*
	 * Breakpoint insertion should be done in an idempotent way,
	 * so if the breakpoint to given address is already inserted,
	 * we will not insert a new breakpoint and we will not consider
	 * this as faulty behaviour.
	 */
	debug_breakpoint_t *debug_breakpoint = NULL;
	for_each(debugger_breakpoints, debug_breakpoint, debug_breakpoint_t) {
		if (debug_breakpoint->pc == addr) {
			return;
		}
	}
	
	/* Breakpoint is not inserted so do it now. */
	
	debug_breakpoint_t *inserted_breakpoint =
	    (debug_breakpoint_t *) safe_malloc_t(debug_breakpoint_t);
	
	item_init(&inserted_breakpoint->item);
	inserted_breakpoint->pc = addr;
	
	list_append(&debugger_breakpoints, &inserted_breakpoint->item);
}

static void gdb_remove_debugger_breakpoint(ptr_t addr)
{
	debug_breakpoint_t *debug_breakpoint = NULL;
	
	for_each(debugger_breakpoints, debug_breakpoint, debug_breakpoint_t) {
		if (debug_breakpoint->pc == addr) {
			list_remove(&debugger_breakpoints, &debug_breakpoint->item);
		}
	}
}

static void gdb_breakpoint(char* buf, bool insert) 
{
	ptr_t addr;
	uint32_t length;
	
	char *arguments = buf + 3;
	
	switch (buf[1]) {
	case '1':
		sscanf(arguments , "%08x,%x", &addr, &length);
		if (length != INT32) {
			strcpy(buf, "E97");
			return;
		}
		
		if (insert)
			gdb_insert_debugger_breakpoint(addr);
		else
			gdb_remove_debugger_breakpoint(addr);
		
		strcpy(buf, "OK");
		break;
	default:
		/* The rest is not supported */
		strcpy(buf, "");
		break;
	}
}

/** Cancel remote gdb
 *
 * Fail gets whether communication fails (io error)
 *
 * @param rq Detect remote request to close connection
 *
 */
static void gdb_remote_done(bool fail, bool rq)
{
	if (!fail) {
		if (rq)
			gdb_send_message("OK");
		else
			gdb_send_message("W00");
	}
	
	if (close(gdb_fd) == -1)
		io_error("gdb_fd");
	
	gdb_fd = 0;
	
	remote_gdb = false;
	remote_gdb_conn = false;
	
	debug_breakpoint_t *debug_breakpoint;
	for_each(debugger_breakpoints, debug_breakpoint, debug_breakpoint_t) {
		list_remove(&debugger_breakpoints, &debug_breakpoint->item);
	}
}

/** gdb main message loop
 *
 */
void gdb_session(void)
{
	char buf[GDB_BUFFER_SIZE];
	
	while (1) {
		if (remote_gdb_step) {
			remote_gdb_step = false;
			gdb_handle_event(GDB_EVENT_BREAKPOINT);
		}
		
		if (!gdb_get_message(buf)) {
			gdb_remote_done(true, false);
			return;
		}
		
		mprintf("<- %s\n", buf);
		
		switch (buf[0]) {
		case '?':
			sprintf(buf, "S%02x", 0);  /* Event id */
			break;
		case 'H': /* Only one CPU supported currently */
			if (strcmp(buf, "Hc-1") == 0) {
				strcpy(buf, "OK");
				break;
			}
			
			strcpy(buf, "E99");
			break;
		case 'g':  /* Get registers */
			gdb_read_registers(buf);
			break;
		case 'G':
			if (gdb_write_registers(&buf[1]))
				strcpy(buf, "OK");
			else
				strcpy(buf, "E97");
			break;
		case 'm':
			gdb_cmd_mem_operation(buf, true);
			break;
		case 'M':
			gdb_cmd_mem_operation(buf, false);
			break;
		case 'c':
		case 's':
			gdb_cmd_step(buf, buf[0] != 'c');
			return;
		case 'D':
			mprintf("detach...\n");
			gdb_remote_done(false, true);
			return;
		case 'q':  /* Query */
			gdb_process_query(buf);
			break;
		case 'Z':  /* Breakpoint */
		case 'z':
			gdb_breakpoint(buf, (buf[0] == 'Z'));
			break;
		default:
			/* Send "not implemented" */
			strcpy(buf, "");
			break;
		}
		
		gdb_send_message(buf);
	}
}


/** Establish a connection with remote gdb on selected port
 *
 * Traditional call = socket - bind - listen - accept.
 *
 * @return true if succesful
 *
 */
bool gdb_remote_init(void)
{
	list_init(&debugger_breakpoints);
	
	int yes = 1;
	struct sockaddr_in sa_gdb;
	struct sockaddr_in sa_srv;
	socklen_t addrlen = sizeof(sa_gdb);
	
	gdb_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gdb_fd < 0) {
		io_error("socket");
		return false;
	}
	
	if (setsockopt(gdb_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof(yes))) {
		io_error("setsockopt");
		return false;
	}
	
	memset(&sa_srv, sizeof(sa_srv), 0);
	
	sa_srv.sin_family = AF_INET;
	sa_srv.sin_addr.s_addr = htonl(INADDR_ANY);
	sa_srv.sin_port = htons(remote_gdb_port);
	
	if (bind(gdb_fd, (struct sockaddr *) &sa_srv, sizeof(sa_srv)) < 0) {
		io_error("bind");
		return false;
	}
	
	if (listen(gdb_fd, 1) < 0) {
		io_error("listen");
		return false;
	}
	
	mprintf("Waiting for GDB response on %d...\n", remote_gdb_port);
	
	gdb_fd = accept(gdb_fd, (struct sockaddr *) &sa_gdb, &addrlen);
	if (gdb_fd < 0) {
		if (errno == EINTR)
			mprintf("...interrupted.\n");
		else
			io_error("accept");
		return false;
	}
	
	mprintf("...done\n");
	
	return true;
}
