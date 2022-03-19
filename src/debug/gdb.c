/*
 * Copyright (c) 2003 Viliam Holub
 * Copyright (c) 2010 Tomas Martinec
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *   Remote debugging
 *
 * The main function is the gdb_session, which waits for the commands
 * from debugger, performs them and eventually resumes the simulation.
 * Communication from the debugger can be also initiated by gdb_handle_event,
 * which is typically called, when a debugger breakpoint is hit.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>
#include "breakpoint.h"
#include "gdb.h"
#include "../arch/network.h"
#include "../device/cpu/mips_r4000/cpu.h"
#include "../device/cpu/riscv_rv32ima/cpu.h"
#include "../device/cpu/general_cpu.h"
#include "../device/dr4kcpu.h"
#include "../assert.h"
#include "../endian.h"
#include "../fault.h"
#include "../main.h"
#include "../parser.h"
#include "../text.h"
#include "../utils.h"

#ifdef GDB_DEBUG

#define gdb_debug(fmt, ...) \
	fprintf(stderr, (fmt), ##__VA_ARGS__)

#else /* GDB_DEBUG */

#define gdb_debug(fmt, ...)

#endif /* GDB_DEBUG */


#define MAX_BAD_CHECKSUMS  10
#define GRANULARITY        1024

#define GDB_NOT_SUPPORTED             ""
#define GDB_REPLY_OK                  "OK"
#define GDB_REPLY_WARNING             "W00"
#define GDB_REPLY_BAD_MEMORY_COMMAND  "E00"
#define GDB_REPLY_MEMORY_WRITE_FAIL   "E01"
#define GDB_REPLY_MEMORY_READ_FAIL    "E02"
#define GDB_REPLY_BAD_BREAKPOINT      "E04"
#define GDB_REPLY_REGISTER_WRITE_FAIL "E05"

#define GDB_REGISTER_STATUS         32
#define GDB_REGISTER_MULTIPLY_LOW   33
#define GDB_REGISTER_MULTIPLY_HIGH  34
#define GDB_REGISTER_BAD            35
#define GDB_REGISTER_CAUSE          36
#define GDB_REGISTER_PC             37

typedef union {
	uint8_t uint8[8];
	uint64_t uint64;
} __attribute__((packed)) union64_t;

static int gdb_fd = -1;
static unsigned int cpuno_global = 0;
static unsigned int cpuno_step = 0;

/** Read one character from gdb remote descriptor.
 *
 * The bit 7 is cleared.
 *
 * @param c Character read.
 *
 * @return True if successful.
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

/** Write one character to gdb remote descriptor.
 *
 * @param c Char to be written.
 *
 * @return True if successful.
 *
 */
static bool gdb_safe_write(char c)
{
	ssize_t written = write(gdb_fd, &c, 1);
	if (written == -1) {
		io_error("gdb");
		return false;
	}
	
	return true;
}

/** Read request from gdb and test for correctness
 *
 * @return Allocated request buffer or NULL on failture.
 *
 */
static char *gdb_get_request(void)
{
	string_t req;
	string_init(&req);
	
	unsigned int i;
	
	for (i = 0; i < MAX_BAD_CHECKSUMS; i++) {
		string_clear(&req);
		
		/*
		 * Message starts with a $ character,
		 * ignore everything before it.
		 */
		char c = 0;
		while (c != '$') {
			if (!gdb_safe_read(&c))
				return NULL;
		}
		
		/*
		 * Read the message characters until
		 * a # character is found.
		 */
		uint8_t checksum = 0;
		
		c = 0;
		while (c != '#') {
			if (!gdb_safe_read(&c)) {
				string_done(&req);
				return NULL;
			}
			
			if (c != '#') {
				string_push(&req, c);
				checksum += c;
			}
		}
		
		/* Get checksum */
		string_t checksum_str;
		string_init(&checksum_str);
		
		if (!gdb_safe_read(&c)) {
			string_done(&req);
			string_done(&checksum_str);
			return NULL;
		}
		string_push(&checksum_str, c);
		
		if (!gdb_safe_read(&c)) {
			string_done(&req);
			string_done(&checksum_str);
			return NULL;
		}
		string_push(&checksum_str, c);
		
		unsigned int req_checksum = 0;
		sscanf(checksum_str.str, "%02x", &req_checksum);
		string_done(&checksum_str);
		
		if (checksum == req_checksum)
			break;
		
		/* Checksum error, ask for re-send */
		if (!gdb_safe_write('-')) {
			string_done(&req);
			return NULL;
		}
	}
	
	if (i >= MAX_BAD_CHECKSUMS) {
		error("Communication checksum failure %u times (read)",
		    MAX_BAD_CHECKSUMS);
		return NULL;
	}
	
	/* Send acknowledgement */
	if (!gdb_safe_write('+')) {
		string_done(&req);
		return NULL;
	}
	
	gdb_debug("<- %s\n", req.str);
	return req.str;
}

/** Send reply to gdb
 *
 * @param reply NULL terminated reply to be sent.
 *
 * @return True if the reply has been sent successfully.
 *
 */
static bool gdb_send_reply(char *reply)
{
	gdb_debug("-> %s\n", reply);
	
	unsigned int i;
	
	for (i = 0; i < MAX_BAD_CHECKSUMS; i++) {
		/* Initial character */
		if (!gdb_safe_write('$'))
			return false;
		
		/* Message */
		
		uint8_t checksum = 0;
		size_t pos = 0;
		
		while (reply[pos] != 0) {
			if (!gdb_safe_write(reply[pos]))
				return false;
			
			checksum += reply[pos];
			pos++;
		}
		
		/* Ending character and the checksum */
		
		if (!gdb_safe_write('#'))
			return false;
		
		if (!gdb_safe_write(hexchar[checksum >> 4]))
			return false;
		
		if (!gdb_safe_write(hexchar[checksum & 0x0f]))
			return false;
		
		char c;
		if (!gdb_safe_read(&c))
			return false;
		
		if (c == '+')
			break;
	}
	
	if (i >= MAX_BAD_CHECKSUMS) {
		error("Communication checksum failure %u times (write)",
		    MAX_BAD_CHECKSUMS);
		return false;
	}
	
	return true;
}

/** Read length bytes from address of machine memory
 *
 */
static void gdb_read_physmem(ptr36_t addr, len36_t length)
{
	string_t str;
	string_init(&str);
	
	/*
	 * We read the memory byte by byte. This ensures
	 * that we will send the content of memory with correct
	 * endianess.
	 */
	while (length > 0) {
		uint8_t value = physmem_read8(-1 /*NULL*/, addr, false);
		string_printf(&str, "%02" PRIx8, value);
		
		length--;
		addr++;
	}
	
	gdb_send_reply(str.str);
	string_done(&str);
}

/** Write length bytes to address in memory from hex string
 *
 */
static void gdb_write_physmem(ptr36_t addr, len36_t length, char *data)
{
	while (length > 0) {
		/* Read one byte */
		unsigned int value;
		int matched = sscanf(data, "%02x", &value);
		if (matched != 1) {
			gdb_send_reply(GDB_REPLY_BAD_MEMORY_COMMAND);
			return;
		}
		
		/* Write it */
		if (!physmem_write8(-1 /*NULL*/, addr, (uint8_t) value, false)) {
			gdb_send_reply(GDB_REPLY_MEMORY_WRITE_FAIL);
			return;
		}
		
		data += 2;
		length--;
		addr++;
	}
	
	gdb_send_reply(GDB_REPLY_OK);
}

/** Dump one register into given buffer in hex
 *
 */
static void gdb_register_dump(string_t *str, uint64_t val)
{
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
	
	union64_t value;
	value.uint64 = convert_uint64_t_endian(val);
	
	string_printf(str, "%02x%02x%02x%02x%02x%02x%02x%02x",
	    value.uint8[0], value.uint8[1],
	    value.uint8[2], value.uint8[3],
	    value.uint8[4], value.uint8[5],
	    value.uint8[6], value.uint8[7]);
}

/** Dump given count of registers into given buffer in hex
 *
 *
 */
static void gdb_registers_dump(string_t *str, reg64_t *regs,
    unsigned int count)
{
	unsigned int i;
	
	for (i = 0; i < count; i++)
		gdb_register_dump(str, regs[i].val);
}

/** Write new value of one register from given hex string
 *
 * @param data Buffer containing the hex string. The pointer
 *             is modified to point to the end of written part.
 * @param val  Pointer to value.
 *
 * @return True if the hex string was in the correct form.
 *
 */
static bool gdb_register_upload(char **data, uint64_t *val)
{
	unsigned int values[8];
	
	/* Read 4 bytes */
	int matched = sscanf(*data, "%02x%02x%02x%02x%02x%02x%02x%02x",
	    &values[0], &values[1], &values[2], &values[3],
	    &values[4], &values[5], &values[6], &values[7]);
	
	if (matched != 8) {
		gdb_send_reply(GDB_REPLY_REGISTER_WRITE_FAIL);
		return false;
	}
	
	/* Convert it to uint32_t and handle the endianness */
	union64_t value;
	value.uint8[0] = values[0];
	value.uint8[1] = values[1];
	value.uint8[2] = values[2];
	value.uint8[3] = values[3];
	value.uint8[4] = values[4];
	value.uint8[5] = values[5];
	value.uint8[6] = values[6];
	value.uint8[7] = values[7];
	
	*val = convert_uint64_t_endian(value.uint64);
	*data += 16;
	return true;
}

/** Write new value of registers from given hex string
 *
 * @param data Buffer containing the hex string. The pointer
 *             is modified to point to the end of written part.
 * @param regs  Array of registers to change.
 * @param count Number of registers to change.
 *
 * @return True if the hex string was in the correct form.
 *
 */
static bool gdb_registers_upload(char **data, reg64_t *regs,
    unsigned int count)
{
	unsigned int i;
	
	for (i = 0; i < count; i++) {
		if (!gdb_register_upload(data, &regs[i].val))
			return false;
	}
	
	return true;
}

/** Notify the debugger about an event
 *
 * Send the event number and current PC to the debugger
 * and make the simulator wait for next command from the debugger.
 *
 * @param event Signal value, which specifies what happened.
 *
 */
void gdb_handle_event(gdb_event_t event)
{
	string_t msg;
	string_init(&msg);
	
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 

	string_printf(&msg, "T%02x%02x:", event, GDB_REGISTER_PC);
	gdb_register_dump(&msg, cpu->pc.ptr);
	string_push(&msg, ';');
	
	gdb_send_reply(msg.str);
	string_done(&msg);
	
	remote_gdb_listen = true;
}

/** Read register contents
 *
 * Read register contents and send it in a suitable
 * format for the debugger.
 *
 */
static void gdb_read_registers(void)
{
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 
	string_t str;
	string_init(&str);
	
	gdb_registers_dump(&str, cpu->regs, 32);
	gdb_register_dump(&str, cpu->cp0[cp0_Status].val);
	gdb_register_dump(&str, cpu->loreg.val);
	gdb_register_dump(&str, cpu->hireg.val);
	gdb_register_dump(&str, cpu->cp0[cp0_BadVAddr].val);
	gdb_register_dump(&str, cpu->cp0[cp0_Cause].val);
	gdb_register_dump(&str, cpu->pc.ptr);
	
	gdb_send_reply(str.str);
	string_done(&str);
}

/** Set new content of registers
 *
 * Set new content of registers according to the hex string
 * from the debugger.
 *
 * @param req Debugger request.
 *
 */
static void gdb_write_registers(char *req)
{
	char *query = req + 1;
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 
	
	if (!gdb_registers_upload(&query, cpu->regs, 32))
		return;
	
	if (!gdb_register_upload(&query, &cpu->cp0[cp0_Status].val))
		return;
	
	if (!gdb_register_upload(&query, &cpu->loreg.val))
		return;
	
	if (!gdb_register_upload(&query, &cpu->hireg.val))
		return;
	
	if (!gdb_register_upload(&query, &cpu->cp0[cp0_BadVAddr].val))
		return;
	
	if (!gdb_register_upload(&query, &cpu->cp0[cp0_Cause].val))
		return;
	
	if (!gdb_register_upload(&query, &cpu->pc.ptr))
		return;
	
	gdb_send_reply(GDB_REPLY_OK);
}

/** Read or write memory
 *
 * @param req  Whole debugger request.
 * @param read True to read from the memory.
 *
 */
static void gdb_cmd_mem_operation(char *req, bool read)
{
	char *query = req + 1;
	
	/* Parse the query */
	unsigned int address = 0;
	unsigned int length = 0;
	int matched = sscanf(query, "%x,%d", &address, &length);
	if (matched != 2) {
		gdb_send_reply(GDB_NOT_SUPPORTED);
		return;
	}
	
	/* Addresses are physical */
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 
	
	ptr64_t virt;
	virt.ptr = address;
	len64_t len = length;
	ptr36_t phys;
	
	if (r4k_convert_addr(cpu, virt, &phys, false, false) == excNone) {
		if (!read) {
			/* Move the pointer to the data to be written */
			query = strchr(query, ':');
			if (query == NULL) {
				gdb_send_reply(GDB_REPLY_BAD_MEMORY_COMMAND);
				return;
			}
			
			query++;
			gdb_write_physmem(phys, len, query);
		} else
			gdb_read_physmem(phys, len);
	} else {
		if (!read)
			gdb_send_reply(GDB_REPLY_MEMORY_WRITE_FAIL);
		else
			gdb_send_reply(GDB_REPLY_MEMORY_READ_FAIL);
	}
}

/** Step or continue command from the debugger
 *
 * @param req  Buffer, that can contain resume address.
 * @param step True for single stepping, false for continue.
 *
 */
static void gdb_cmd_step(char *req, bool step)
{
	char *query = req + 1;
	
	/*
	 * Decode address at which the processor should
	 * resume.  If not specified, use the current PC.
	 * How is this useful?
	 */
	unsigned int address;
	int matched = sscanf(query, "%x", &address);
	if (matched == 1) {
		ptr64_t addr;
		addr.ptr = address;
		//TODO: implement for both
		r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
		// TODO: ASSERT that it really us r4k 
		r4k_set_pc(cpu, addr);
	}
	
	remote_gdb_step = step;
	remote_gdb_listen = step;
}

/** Process debugger query
 *
 */
static void gdb_process_query(char *req)
{
	char *query = req + 1;
	
	if (strncmp(query, "Supported", 9) == 0) {
		/* No extensions are supported */
		gdb_send_reply(GDB_NOT_SUPPORTED);
		return;
	}
	
	if (strcmp(query, "C") == 0) {
		char reply[5];
		
		ASSERT(cpuno_global < MAX_CPUS);
		
		/* Represent processors as threads */
		snprintf(reply, 5, "QC%02x", cpuno_global);
		gdb_send_reply(reply);
		return;
	}
	
	if (strcmp(query, "Attached") == 0) {
		/*
		 * We pretend that we have attached to aprocess,
		 * gdb will then try to detach when quiting.
		 */
		gdb_send_reply("1");
		return;
	}
	
	if (strcmp(query, "Offsets") == 0) {
		/* There are no relocations known to the simulator */
		gdb_send_reply("Text=0;Data=0;Bss=0");
		return;
	}
	
	if (strcmp(query, "Symbol::") == 0) {
		/* Symbol lookup, we do not need it -> send OK */
		gdb_send_reply(GDB_REPLY_OK);
		return;
	}
	
	if (strcmp(query, "TStatus") == 0) {
		/* Tracepoints are not supported */
		gdb_send_reply("T0");
		return;
	}
	
	/* Unsupported query */
	gdb_send_reply(GDB_NOT_SUPPORTED);
}

static unsigned int gdb_decode_threadid(char *threadid)
{
	if ((strcmp(threadid, "-1") == 0)
	    || (strcmp(threadid, "0") == 0)) {
		gdb_send_reply(GDB_REPLY_OK);
		return 0;
	}
	
	// TODO: decode specific threadids
	gdb_send_reply(GDB_NOT_SUPPORTED);
	return 0;
}

/** Process debugger thread selection
 *
 */
static void gdb_process_thread(char *req)
{
	char *query = req + 1;
	
	switch (query[0]) {
	case 'g':
		cpuno_global = gdb_decode_threadid(query + 1);
		break;
	case 'c':
		cpuno_step = gdb_decode_threadid(query + 1);
		break;
	default:
		/* Unsupported kind */
		gdb_send_reply(GDB_NOT_SUPPORTED);
	}
}

static void gdb_reply_event(gdb_event_t event)
{
	string_t reply;
	string_init(&reply);
	
	string_printf(&reply, "S%02x", event);
	gdb_send_reply(reply.str);
	
	string_done(&reply);
}

/** Activate code breakpoint
 *
 */
static void gdb_insert_code_breakpoint(r4k_cpu_t *cpu, ptr64_t addr)
{
	/*
	 * Breakpoint insertion should be done in an idempotent way,
	 * so if the breakpoint to given address is already inserted,
	 * we will not insert a new breakpoint and we will not consider
	 * this as faulty behavior.
	 */
	breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps,
	    addr, BREAKPOINT_FILTER_DEBUGGER);
	
	if (breakpoint != NULL)
		return;
	
	/* Breakpoint not found, thus insert it now. */
	breakpoint_t *inserted_breakpoint =
	    breakpoint_init(addr, BREAKPOINT_KIND_DEBUGGER);
	
	list_append(&cpu->bps, &inserted_breakpoint->item);
}

/** Deactivate code breakpoint
 *
 */
static void gdb_remove_code_breakpoint(r4k_cpu_t *cpu, ptr64_t addr)
{
	breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps,
	    addr, BREAKPOINT_FILTER_DEBUGGER);
	
	/* Removing non existent breakpoint is not considered as a bug */
	if (breakpoint == NULL)
		return;
	
	list_remove(&cpu->bps, &breakpoint->item);
	safe_free(breakpoint);
}

/** Handle code or memory breakpoint commands from the debugger
 *
 * @param req    Request from the debugger.
 * @param insert True if the breakpoint should be inserted.
 *
 */
static void gdb_breakpoint(char *req, bool insert)
{
	/* Decode the type of breakpoint (code or memory)
	   and memory access conditions */
	
	bool code_breakpoint = false;
	access_filter_t memory_access = ACCESS_FILTER_ANY;
	
	switch (req[1]) {
	case '0':  /* Software breakpoint */
	case '1':  /* Hardware breakpoint */
		code_breakpoint = true;
		break;
	case '2':  /* Memory breakpoint (write) */
		code_breakpoint = false;
		memory_access = ACCESS_FILTER_WRITE;
		break;
	case '3':  /* Memory breakpoint (read) */
		code_breakpoint = false;
		memory_access = ACCESS_FILTER_READ;
		break;
	case '4':  /* Memory breakpoint (any) */
		code_breakpoint = false;
		memory_access = ACCESS_FILTER_ANY;
		break;
	default:
		/* The rest is not supported */
		gdb_send_reply(GDB_NOT_SUPPORTED);
		return;
	}
	
	/* Decode the breakpoint address and length */
	char *arguments = req + 2;
	unsigned int address;
	unsigned int length;
	int matched = sscanf(arguments, ",%x,%x", &address, &length);
	
	if (matched != 2) {
		gdb_send_reply(GDB_REPLY_BAD_BREAKPOINT);
		return;
	}
	
	ptr64_t virt;
	// Extend the address as the GDB sends the address in 32bits.
	virt.ptr = UINT64_C(0xffffffff00000000) | address;
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 
	
	if (code_breakpoint) {
		if (length != 4) {
			gdb_send_reply(GDB_REPLY_BAD_BREAKPOINT);
			return;
		}
		
		if (insert)
			gdb_insert_code_breakpoint(cpu, virt);
		else
			gdb_remove_code_breakpoint(cpu, virt);
	} else {
		ptr36_t phys;
		
		if (r4k_convert_addr(cpu, virt, &phys, false, false) == excNone) {
			if (insert)
				physmem_breakpoint_add(phys, length,
				    BREAKPOINT_KIND_DEBUGGER, memory_access);
			else
				physmem_breakpoint_remove(phys);
		} else {
			gdb_send_reply(GDB_REPLY_BAD_BREAKPOINT);
			return;
		}
	}
	
	gdb_send_reply(GDB_REPLY_OK);
}

/** Cancel remote debugger connection
 *
 */
static void gdb_remote_done(bool fail, bool remote_request)
{
	//TODO: implement for both
	r4k_cpu_t *cpu = (r4k_cpu_t *)get_cpu(cpuno_global)->data;
	// TODO: ASSERT that it really us r4k 
	
	if (!fail)
		gdb_send_reply(remote_request ? GDB_REPLY_OK : GDB_REPLY_WARNING);
	
	if (close(gdb_fd) == -1)
		io_error("gdb_fd");
	
	gdb_fd = -1;
	cpuno_global = 0;
	cpuno_step = 0;
	
	remote_gdb = false;
	remote_gdb_conn = false;
	
	/* Remove all the debugger breakpoints. */
	breakpoint_t *breakpoint = (breakpoint_t *) cpu->bps.head;
	while (breakpoint != NULL) {
		breakpoint_t *removed = breakpoint;
		breakpoint = (breakpoint_t *) breakpoint->item.next;
		
		list_remove(&cpu->bps, &removed->item);
		safe_free(removed);
	}
	
	physmem_breakpoint_remove_filtered(BREAKPOINT_FILTER_DEBUGGER);
}

/** Gdb main message loop implementation.
 *
 * Wait for commands from the debugger and handle them. This function
 * will return if, the message from the debugger was not correctly received,
 * step or continue command was received or the debugger has detached.
 *
 */
void gdb_session(void)
{
	/* At first send the result of single step command */
	if (remote_gdb_step) {
		remote_gdb_step = false;
		gdb_handle_event(GDB_EVENT_BREAKPOINT);
	}
	
	while (true) {
		/* Read the command. */
		char *req = gdb_get_request();
		if (req == NULL) {
			gdb_remote_done(true, false);
			return;
		}
		
		switch (req[0]) {
		case 'H':  /* Thread selection */
			gdb_process_thread(req);
			break;
		case '?':  /* Get current status */
			gdb_reply_event(GDB_EVENT_NO_EXCEPTION);
			break;
		case 'g':  /* Read registers */
			gdb_read_registers();
			break;
		case 'G':  /* Write registers */
			gdb_write_registers(req);
			break;
		case 'm':  /* Memory read */
			gdb_cmd_mem_operation(req, true);
			break;
		case 'M':  /* Memory write */
			gdb_cmd_mem_operation(req, false);
			break;
		case 'c':  /* Continue */
			gdb_cmd_step(req, false);
			return;
		case 's':  /* Step */
			gdb_cmd_step(req, true);
			return;
		case 'D':  /* Detach */
			alert("GDB: Detached");
			gdb_remote_done(false, true);
			return;
		case 'q':  /* Query */
			gdb_process_query(req);
			break;
		case 'Z':  /* Add breakpoint */
			gdb_breakpoint(req, true);
			break;
		case 'z':  /* Remove breakpoint */
			gdb_breakpoint(req, false);
			break;
		default:
			/* Send "not implemented" */
			gdb_send_reply(GDB_NOT_SUPPORTED);
			break;
		}
		
		safe_free(req);
	}
}

/** Establish a connection with remote gdb on selected port
 *
 * Traditional call = socket - bind - listen - accept.
 *
 * @return True if successful
 *
 */
bool gdb_remote_init(void)
{
	gdb_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gdb_fd < 0) {
		io_error("socket");
		return false;
	}
	
	int yes = 1;
	if (setsockopt(gdb_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof(yes))) {
		io_error("setsockopt");
		return false;
	}
	
	struct sockaddr_in sa_srv;
	memset(&sa_srv, 0, sizeof(sa_srv));
	
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
	
	alert("GDB: Waiting for connection on port %u", remote_gdb_port);
	
	struct sockaddr_in sa_gdb;
	socklen_t addrlen = sizeof(sa_gdb);
	gdb_fd = accept(gdb_fd, (struct sockaddr *) &sa_gdb, &addrlen);
	if (gdb_fd < 0) {
		if (errno == EINTR)
			alert("GDB: Interrupted");
		else
			io_error("accept");
		
		return false;
	}
	
	alert("GDB: Connected");
	
	return true;
}
