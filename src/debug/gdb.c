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
#include <sys/types.h>

#include "../arch/network.h"
#include "../main.h"
#include "gdb.h"
#include "../check.h"
#include "breakpoint.h"
#include "../text.h"
#include "../mtypes.h"
#include "../fault.h"
#include "../io/output.h"
#include "../device/machine.h"
#include "../parser.h"
#include "../utils.h"
#include "../cpu/cpu.h"

#include "../device/dcpu.h"

static int gdb_fd = 0;

/** GDB communication debugging
 *
 * Print communication between simulator and debugger. It can
 * be turned on by defining the GDB_DEBUG_PRINTS macro.
 *
 * @param fmt Standard printf formating string.
 *
 */
static void gdb_debug_print(const char *fmt, ...)
{
#ifdef GDB_DEBUG_PRINTS
	PRE(fmt != NULL);
	
	va_list ap;
	
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
#endif
}

/** Read length bytes from address for machine memory
 *
 * Converts to hex and writes to buf.
 *
 * @param address Starting address for reading the memory.
 * @param length  Number of bytes to read.
 * @param buf     Buffer of at least GDB_BUFFER_SIZE bytes for storing
 *                the memory content.
 *
 * @return False if the content of the memory is too long.
 *
 */
static bool gdb_read_mem(ptr_t address, size_t length, char *buf)
{
	char* start_of_buffer = buf;
	
	/*
	 * We read the memory byte by byte. This ensures
	 * that we will send the content of memory with correct
	 * endianess.
	 */
	while (length > 0) {
		if (buf - start_of_buffer >= GDB_BUFFER_SIZE)
			return false;
		
		uint32_t byte_value = mem_read(NULL, address, BITS_8, false);
		sprintf(buf, "%02x", byte_value);
		
		buf += 2;
		length -= 1;
		address += 1;
	}
	
	return true;
}

/** Write length bytes to address in memory from hex string
 *
 * @param addr   Address of memory, where the data will be written.
 * @param length Count of bytes to write.
 * @param buf    Hex string containing data to be written. Each byte
 *               has %02x format.
 *
 * @return True if the hex string was correct and the data were successfully
 *         written.
 *
 */
static bool gdb_write_mem(ptr_t addr, size_t length, char *buf)
{
	while (length > 0) {
		
		/* Read one byte */
		unsigned int value;
		int matched = sscanf(buf, "%02x", &value);
		if (matched != 1)
			return false;
		
		/* Write it */
		if (!mem_write(NULL, addr, value, BITS_8, false))
			return false;
		
		buf += 2;
		length -= 1;
		addr++;
	}
	
	return true;
}

/** Read one character from gdb remote descriptor.
 *
 * The bit 7 is cleared.
 *
 * @param c Read char.
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
	gdb_debug_print("%c", c);
	
	ssize_t written = write(gdb_fd, &c, 1);
	if (written == -1) {
		io_error("gdb");
		return false;
	}
	
	return true;
}

/** Read a message from gdb and test for correctness
 *
 * @param data Buffer for the message, must be at least GDB_BUFFER_SIZE
 *             bytes long.
 *
 * @return False, if there was an error.
 *
 */
static bool gdb_get_message(char *data)
{
	unsigned int i;
	
	for (i = 0; i < GDB_MAXIMUM_BAD_CHECKSUMS_COUNT; i++) {
		
		/*
		 * Message starts with a $ character,
		 * ignore everything before it.
		 */
		char c = 0;
		while (c != '$') {
			if (!gdb_safe_read(&c))
				return false;
		}
		
		/*
		 * Read the message characters until
		 * a # character is found.
		 */
		char *buf = data;
		uint8_t counted_checksum = 0;
		size_t read_count = 0;
		
		while (read_count < GDB_BUFFER_SIZE) {
			if (!gdb_safe_read(&c))
				return false;
			
			if (c == '#')
				break;
			
			*buf = c;
			buf++;
			counted_checksum += c;
			read_count++;
		}
		
		*buf = 0;
		if (read_count >= GDB_BUFFER_SIZE) {
			error("Message to debugger is too long, data:\n%s\n", buf);
			return false;
		}
		
		/* Compute checksum */
		char expected_checksum_buf[2 + 1];
		if (!gdb_safe_read(expected_checksum_buf))
			return false;
		
		if (!gdb_safe_read(expected_checksum_buf + 1))
			return false;
		
		expected_checksum_buf[2] = 0;
		
		unsigned int expected_checksum = 0;
		sscanf(expected_checksum_buf, "%02x", &expected_checksum);
		
		if (counted_checksum == expected_checksum)
			break;
		
		/* Checksum error, ask for re-sending */
		if (!gdb_safe_write('-'))
			return false;
	}
	
	if (i >= GDB_MAXIMUM_BAD_CHECKSUMS_COUNT) {
		error("Messages from the debugger had bad checksum too many times\n");
		return false;
	}
	
	/* Send acknowledgement */
	if (!gdb_safe_write('+'))
		return false;
	
	return true;
}

/** Send a message to gdb
 *
 * @param message NULL terminated message to be sent
 *
 * @return True the message has been sent successfully
 *
 */
static bool gdb_send_message(char *message)
{
	unsigned int i;
	
	for (i = 0; i < GDB_MAXIMUM_BAD_CHECKSUMS_COUNT; i++) {
		gdb_debug_print("->");
		
		/* Initial character */
		
		if (!gdb_safe_write('$'))
			return false;
		
		/* Message itself */
		
		unsigned char counted_checksum = 0;
		
		char* to_be_sent_part = message;
		while (*to_be_sent_part != 0) {
			if (!gdb_safe_write(*to_be_sent_part))
				return false;
			
			counted_checksum += *to_be_sent_part;
			to_be_sent_part++;
		}
		
		/* Ending character and the checksum */
		
		if (!gdb_safe_write('#'))
			return false;
		
		if (!gdb_safe_write(hexchar[counted_checksum >> 4]))
			return false;
		
		if (!gdb_safe_write(hexchar[counted_checksum % 16]))
			return false;
		
		gdb_debug_print("\n");
		
		char c;
		if (!gdb_safe_read(&c))
			return false;
		
		if (c == '+')
			break;
	}
	
	if (i >= GDB_MAXIMUM_BAD_CHECKSUMS_COUNT) {
		error("Messages to the debugger was resent too many times\n");
		return false;
	}
	
	return true;
}

/** Dump given count of registers into given buffer in hex
 *
 * @param registers Array of registers to be dumped
 * @param buf       Buffer for the created hex string
 * @param count     Count of registers to be dumped
 *
 * @param Pointer to terminating null at the and of created hex string.
 *
 */
static char *gdb_registers_dump(const uint32_t *registers, char *buf,
    size_t count)
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

/** Dump one register into given buffer in hex
 *
 * @param register_value Value of dumped register
 * @param buf            Buffer for the created hex string
 *
 * @param Pointer to terminating null at the and of created hex string.
 *
 */
static char *gdb_register_dump(uint32_t register_value, char *buf)
{
	return gdb_registers_dump(&register_value, buf, 1);
}

/** Writes new value of registers from given hex string
 *
 * @param buf       Buffer containing the hex string. The pointer is modified to point
 *                  to the end of written part.
 * @param registers Array of registers, which will be changed.
 * @param count     Count of registers, which will be changed.
 *
 * @return True, if the hex string was in the correct form.
 *
 */
static bool gdb_registers_upload(char **buf, uint32_t *registers, size_t count)
{
	size_t i;
	
	for (i = 0; i < count; i++) {
		unsigned int values[4];
		
		/* Read 4 bytes */
		int matched = sscanf(*buf, "%02x%02x%02x%02x", &values[0],
		    &values[1], &values[2], &values[3]);
		
		if (matched != 4)
			return false;
		
		/* Convert it to uint32 and handle the endianness */
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

/** Writes new value of one register from given hex string
 *
 * @param buf            Buffer containing the hex string. The pointer
 *                       is modified to point to the end of written part.
 * @param register_value Array of registers, which will be changed.
 *
 * @return True, if the hex string was in the correct form.
 *
 */
static bool gdb_register_upload(char **buf, uint32_t *register_value)
{
	return gdb_registers_upload(buf, register_value, 1);
}

/** Notify the debugger about an event
 *
 * Sends the event number and current pc to the debugger
 * and makes the simulator wait for next command from the debugger.
 *
 * @param event Signal value, which specifies what happened.
 *
 */
void gdb_handle_event(gdb_events_t event)
{
	char gdb_buf[GDB_BUFFER_SIZE];
	
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	char pc_string[(sizeof(uint32_t) * 2) + 1];
	gdb_register_dump(cpu->pc, pc_string);
	
	sprintf(gdb_buf, "T%02x%02x:%s;", event, GDB_REGISTER_NUMBER_PC, pc_string);
	gdb_send_message(gdb_buf);
	
	remote_gdb_listen = true;
}

/** Read register contents
 *
 * Read register contents and write it to a buffer in a suitable
 * format for the debugger.
 *
 * @param buf Buffer for string which will be sent to the debugger.
 *
 */
static void gdb_read_registers(char *buf)
{
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	buf = gdb_registers_dump(cpu->regs, buf, 32);
	buf = gdb_register_dump(cpu->cp0[cp0_Status], buf);
	buf = gdb_register_dump(cpu->loreg, buf);
	buf = gdb_register_dump(cpu->hireg, buf);
	buf = gdb_register_dump(cpu->cp0[cp0_BadVAddr], buf);
	buf = gdb_register_dump(cpu->cp0[cp0_Cause], buf);
	buf = gdb_register_dump(cpu->pc, buf);
}

/** Set new content of registers
 *
 * Set new content of registers according to the hex string
 * from the debugger.
 *
 * @param buf Hexa string from the debugger.
 *
 * @return True, if the hex string was in the correct form.
 *
 */
static bool gdb_write_registers(char *buf)
{
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	if (!gdb_registers_upload(&buf, cpu->regs, 32))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->cp0[cp0_Status]))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->loreg))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->hireg))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->cp0[cp0_BadVAddr]))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->cp0[cp0_Cause]))
		return false;
	
	if (!gdb_register_upload(&buf, &cpu->pc))
		return false;
	
	return true;
}

/** Read or write memory
 *
 * @param data Whole debugger command
 * @param read True to read from the memory
 *
 */
static void gdb_cmd_mem_operation(char *data, bool read)
{
	data++;
	char *buf = data;
	
	/* Try to read and parse the command */
	ptr_t addr = 0;
	size_t length = 0;
	int matched = sscanf(data, "%x,%d", &addr, &length);
	if (matched != 2) {
		strcpy(buf, GDB_ERROR_STRING_BAD_MEMORY_COMMAND);
		return;
	}
	
	/* We need to get physical address */
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	convert_addr(cpu, &addr, false, false);
	
	/* Move the data pointer to the data to be written */
	if (!read) {
		data = strchr(data, ':');
		if (data == NULL) {
			strcpy(buf, GDB_ERROR_STRING_BAD_MEMORY_COMMAND);
			return;
		}
		
		data++;
	}
	
	/* Perform the memory operation */
	if (!read) {
		/* Try to write the new content to the memory */
		bool success = gdb_write_mem(addr, length, data);
		strcpy(buf, success ? "OK" : GDB_ERROR_STRING_MEMORY_WRITE_FAIL);
		
		return;
	}
	
	/* Try to read the content of the memory */
	if (!gdb_read_mem(addr, length, buf))
		strcpy(buf, GDB_ERROR_STRING_MEMORY_READ_FAIL);
}

/** Step or continue command from the debugger
 *
 * @param data Buffer, that can contain resume address.
 * @param step True for single stepping, false for continue.
 *
 */
static void gdb_cmd_step(char *data, bool step)
{
	data++;
	
	/*
	 * Address at which the processor should resume.
	 * If not specified, use the current pc address.
	 * How is this useful?
	 */
	ptr_t addr;
	int matched = sscanf(data, "%x", &addr);
	if (matched == 1) {
		// TODO: Support more processors
		cpu_t *cpu = dcpu_find_no(0);
		cpu_set_pc(cpu, addr);
	}
	
	remote_gdb_step = step;
	remote_gdb_listen = step;
}

/** Create an answer for the debugger query
 *
 * @param data Whole debugger command. It contains answer for
 *             the query at the end of function call.
 *
 */
static void gdb_process_query(char *data)
{
	char *query_content = data + 1;
	
	/*
	 * We pretend that we have attached to some process,
	 * the gdb will then try to detach when quiting.
	 */
	if (strcmp(query_content, "Attached") == 0) {
		strcpy(data, "1");
		return;
	}
	
	/* Thread ID does not have meaning here */
	if (strcmp(query_content, "C") == 0) {
		strcpy(data, "-1");
		return;
	}
	
	/* There is no relocation of program loaded to the simulator */
	if (strcmp(query_content, "Offsets") == 0) {
		strcpy(data, "Text=0;Data=0;Bss=0");
		return;
	}
	
	/* The supported features list has not been implemented yet */
	if (strcmp(query_content, "Supported") == 0) {
		strcpy(data, GDB_NOT_SUPPORTED_STRING);
		return;
	}
	
	/* Symbol lookup, we do not need it -> send ok */
	if (strcmp(query_content, "Symbol::") == 0) {
		strcpy(data, "OK");
		return;
	}
	
	/* Tracepoints are not supported */
	if (strcmp(query_content, "TStatus") == 0) {
		strcpy(data, "T0");
		return;
	}
	
	strcpy(data, GDB_ERROR_STRING_UNKNOWN_QUERY);
}

/** Activate code breakpoint on processor 0 at given address.
 *
 * @param addr PC value of the code breakpoint.
 */
static void gdb_insert_code_breakpoint(ptr_t address)
{
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	/*
	 * Breakpoint insertion should be done in an idempotent way,
	 * so if the breakpoint to given address is already inserted,
	 * we will not insert a new breakpoint and we will not consider
	 * this as faulty behavior.
	 */
	breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps,
	    address, BREAKPOINT_FILTER_DEBUGGER);

	if (breakpoint != NULL)
		return;
	
	/* Breakpoint is not inserted so do it now. */
	breakpoint_t *inserted_breakpoint = breakpoint_init(address,
	    BREAKPOINT_KIND_DEBUGGER);
	
	list_append(&cpu->bps, &inserted_breakpoint->item);
}

/** Deactivate code breakpoint on processor 0 at given address.
 *
 * @param addr PC value of the code breakpoint.
 *
 */
static void gdb_remove_code_breakpoint(ptr_t addr)
{
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps,
	    addr, BREAKPOINT_FILTER_DEBUGGER);
	
	/* Removing non existent breakpoint is not considered as a bug */
	if (breakpoint == NULL)
		return;
	
	list_remove(&cpu->bps, &breakpoint->item);
	safe_free(breakpoint);
}

/** Handle code or memory breakpoint command from the debugger
 *
 * @param buf    Whole command from the debugger. Contains the answer
 *               at the end of function call.
 * @param insert True, if the breakpoint will be inserted.
 *
 */
static void gdb_breakpoint(char *buf, bool insert)
{
	ptr_t address = 0;
	uint32_t length = 0;
	access_filter_t memory_access = ACCESS_FILTER_WRITE;
	bool code_breakpoint = true;
	
	char *arguments = buf + 3;
	
	/* Find out the type of breakpoint (code or memory) and memory access conditions */
	
	char breakpoint_type_char = buf[1];
	switch (breakpoint_type_char) {
	case '0':  /* We handle the software (0) and hardware (1) breakpoints in the same way */
	case '1':
		code_breakpoint = true;
		break;
	case '2':  /* These are memory breakpoints */
	case '3':
	case '4':
		code_breakpoint = false;
		switch (breakpoint_type_char) {
		case '2':
			memory_access = ACCESS_FILTER_WRITE;
			break;
		case '3':
			memory_access = ACCESS_FILTER_READ;
			break;
		default:
			memory_access = ACCESS_FILTER_ANY;
		}
		break;
	default:
		/* The rest is not supported */
		strcpy(buf, GDB_NOT_SUPPORTED_STRING);
		return;
	}
	
	/* Read the breakpoint address and length */
	
	sscanf(arguments, "%x,%x", &address, &length);
	if (length != BITS_32) {
		strcpy(buf, GDB_ERROR_STRING_BAD_BREAKPOINT_COMMAND);
		return;
	}
	
	strcpy(buf, "OK");
	
	/* Handle code breakpoint */
	
	if (insert && code_breakpoint) {
		gdb_insert_code_breakpoint(address);
		return;
	}
	
	if ((!insert) && code_breakpoint) {
		gdb_remove_code_breakpoint(address);
		return;
	}
	
	/* Handle memory breakpoint */
	
	// TODO: Support more processors
	cpu_t* cpu = dcpu_find_no(0);
	convert_addr(cpu, &address, false, false);
	
	if (insert && (!code_breakpoint)) {
		memory_breakpoint_add(address, BREAKPOINT_KIND_DEBUGGER,
		    memory_access);
		return;
	}
	
	if ((!insert) && (!code_breakpoint)) {
		memory_breakpoint_remove(address);
		return;
	}
}

/** Cancel remote gdb
 *
 * Fail gets whether communication fails (io error)
 *
 * @param rq Detect remote request to close connection
 *
 */
static void gdb_remote_done(bool fail, bool remote_request)
{
	if (!fail) {
		char* message = remote_request ? "OK" : "W00";
		gdb_send_message(message);
	}
	
	if (close(gdb_fd) == -1)
		io_error("gdb_fd");
	
	gdb_fd = 0;
	
	remote_gdb = false;
	remote_gdb_conn = false;
	
	/* Remove all the debugger breakpoints. */
	
	// TODO: Support more processors
	cpu_t *cpu = dcpu_find_no(0);
	
	breakpoint_t *breakpoint = (breakpoint_t *) cpu->bps.head;
	while (breakpoint != NULL) {
		breakpoint_t *removed = breakpoint;
		breakpoint = (breakpoint_t *) breakpoint->item.next;
		
		list_remove(&cpu->bps, &removed->item);
		safe_free(removed);
	}
	
	memory_breakpoint_remove_filtered(BREAKPOINT_FILTER_DEBUGGER);
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
	char buf[GDB_BUFFER_SIZE];
	
	/* At first send the result of single step command */
	if (remote_gdb_step) {
		remote_gdb_step = false;
		gdb_handle_event(GDB_EVENT_BREAKPOINT);
	}
	
	while (true) {
		/* Read the command. */
		if (!gdb_get_message(buf)) {
			gdb_remote_done(true, false);
			return;
		}
		
		gdb_debug_print("<- %s\n", buf);
		
		switch (buf[0]) {
		case '?':
			sprintf(buf, "S%02x", GDB_EVENT_NO_EXCEPTION);
			break;
		case 'g':
			gdb_read_registers(buf);
			break;
		case 'G':
			if (gdb_write_registers(&buf[1]))
				strcpy(buf, "OK");
			else
				strcpy(buf, GDB_ERROR_STRING_REGISTERS_WRITE_FAIL);
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
			mprintf("Debugger detach...\n");
			gdb_remote_done(false, true);
			return;
		case 'q':
			gdb_process_query(buf);
			break;
		case 'Z':
		case 'z':
			gdb_breakpoint(buf, (buf[0] == 'Z'));
			break;
		default:
			/* Send "not implemented" */
			strcpy(buf, GDB_NOT_SUPPORTED_STRING);
			break;
		}
		
		/* Send the answer */
		gdb_send_message(buf);
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
	
	struct sockaddr_in sa_gdb;
	socklen_t addrlen = sizeof(sa_gdb);
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
