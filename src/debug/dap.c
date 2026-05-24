#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../assert.h"
#include "../device/cpu/general_cpu.h"
#include "../fault.h"
#include "../main.h"
#include "../physmem.h"
#include "dap.h"

// be64toh is not on macOS
#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#endif

#define DAP_PREFIX "[DAP] "
#define DAP_ARG_COUNT 3

static int connection_fd = -1;
static uint32_t cpuno_default = 0; // Default CPU device number used

typedef enum dap_request_type {
    /** Request to resume execution. Also used for the initial start. */
    ResumeRequest = 0x01,
    /** Request to pause execution. */
    PauseRequest = 0x02,
    /** Request to terminate execution and exit the simulator. */
    TerminateRequest = 0x03,
    /** Request to step `arg0=cpu` by `arg1=count` instructions. */
    StepRequest = 0x04,

    // Breakpoint requests
    /** Request to set a code breakpoint at `arg0=address`. */
    SetCodeBreakpointRequest = 0x05,
    /** Request to remove a code breakpoint at `arg0=address`. */
    RemoveCodeBreakpointRequest = 0x06,

    /** Request to set a physical memory data breakpoint at `arg0=address` with `arg1=kind` of `arg2=size`. */
    SetDataBreakpointRequest = 0x07,
    /** Request to remove a physical memory data breakpoint at `arg0=address`. */
    RemoveDataBreakpointRequest = 0x08,

    // Register requests
    /** Request to read the value of register `arg1=id` in `arg0=cpu`. */
    ReadGeneralRegisterRequest = 0x09,
    /** Request to write to register `arg1=id` in `arg0=cpu` the value `arg2=value`. */
    WriteGeneralRegisterRequest = 0x0A,

    /** Request to read the value of Control and Status Register (CSR) `arg1=id` in `arg0=cpu`. */
    ReadCsrRequest = 0x0B,
    /** Request to write to Control and Status Register (CSR) `arg1=id` the value `arg2=value` in `arg0=cpu`. */
    WriteCsrRequest = 0x0C,

    /** Request to read the value of the program counter of `arg0=cpu`. */
    ReadPCRequest = 0x0D,
    /** Request to write `arg1=value` to the program counter of `arg0=cpu`. */
    WritePCRequest = 0x0E,

    // Memory requests
    /** Request to read physical memory at `arg0=address`. */
    ReadPhysMemoryRequest = 0x0F,
    /** Request to write `arg1=data` (8 B) to physical memory at `arg0=address`. */
    WritePhysMemoryRequest = 0x10,

    /** Request to read virtual memory at `arg0=address`. */
    ReadVirtMemoryRequest = 0x11,
    /** Request to write `arg1=data` (8 B) to virtual memory at `arg0=address`. */
    WriteVirtMemoryRequest = 0x12,

    /** Request to translate the virtual address `arg0=address` to a physical address. */
    TranslateAddressRequest = 0x13,

    // Interrupt requests
    /** Request to raise interrupt `arg0=id`. */
    RaiseInterruptRequest = 0x14,
    /** Request to clear interrupt `arg0=id`. */
    ClearInterruptRequest = 0x15,

    /** Request to get the value of configuration */
    GetConfigRequest = 0x16,
    /** Request to get CPU-specific information for `arg0=cpu_id` */
    GetCpuInfoRequest = 0x17,
} dap_request_type_t;

typedef enum dap_outbound_category {
    /** Response to a request frame. */
    ResponseCategory = 0x01,
    /** Event frame. */
    EventCategory = 0x02
} dap_outbound_category_t;

typedef enum dap_response_status {
    /** Response to a successful request. */
    StatusOk = 0x01,
    /** Response to a failed request with an unspecified error. */
    StatusUnspecifiedError = 0x02,
    /** Response to an unsupported request with type id `arg0`. */
    StatusUnsupportedRequestError = 0x03,
    /** Response to a failed request with an unknown CPU id `arg0`. */
    StatusCpuNotFoundError = 0x04,
    /** Response to a failed request with an unknown register id `arg0`. */
    StatusRegisterNotFoundError = 0x05,
    /** Response to a failed request with bad memory address `arg0`. */
    StatusBadAddressError = 0x06,
} dap_response_status_t;

typedef enum dap_event_type {
    /** Event indicating that the simulator has terminated. */
    TerminatedEvent = 0x01,
    /** Event indicating that the simulator has paused execution with `arg0=cpu` at `arg1=address` due to `arg2=reason` */
    StoppedAtEvent = 0x02,
} dap_event_type_t;

typedef enum dap_stopped_reason {
    /** Stopped due to a pause. */
    StoppedReasonPaused = 0x01,
    /** Stopped due to hitting a breakpoint. */
    StoppedReasonBreakpoint = 0x02,
    /** Stopped due to stepping. */
    StoppedReasonStep = 0x03,
    /** Stopped due to an interrupt. */
    StoppedReasonInterrupt = 0x04,
} dap_stopped_reason_t;

/** Structure for DAP requests */
typedef struct dap_request {
    dap_request_type_t type;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
} dap_request_t;

/** Structure for DAP responses */
typedef struct dap_response {
    dap_response_status_t type;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
} dap_response_t;

/** Structure for DAP events */
typedef struct dap_event {
    dap_event_type_t type;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
} dap_event_t;

enum {
    INBOUND_FRAME_SIZE = 25, /** Size of a single DAP request frame */
    OUTBOUND_FRAME_SIZE = 26, /** Size of a single DAP response frame */
};

/** Length of an inbound frame is always 25 B = 1 B (type) + 8 B (arg0) + 8 B (arg1) + 8 B (arg2) */
static_assert(INBOUND_FRAME_SIZE == sizeof(uint8_t) + DAP_ARG_COUNT * sizeof(uint64_t), "DAP inbound frame must be exactly 25 bytes long");

/** Length of an outbound frame is always 26 B = 1 B (category) + 1 B (status/type) + 8 B (arg0) + 8 B (arg1) + 8 B (arg2) */
static_assert(OUTBOUND_FRAME_SIZE == sizeof(uint8_t) + sizeof(uint8_t) + DAP_ARG_COUNT * sizeof(uint64_t), "DAP inbound frame must be exactly 26 bytes long");

/** Internal buffer for incoming frames */
static uint8_t frame_buffer[INBOUND_FRAME_SIZE];
static ssize_t frame_buffered_bytes = 0; // Number of bytes currently buffered in frame buffer

bool dap_init(void)
{
    const int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        io_error("dap_socket");
        return false;
    }

    const int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
        io_error("setsockopt");
        return false;
    }

    struct sockaddr_in address = { 0 };
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(dap_port);

    if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
        io_error("dap_bind");
        return false;
    }

    if (listen(sock, 1) < 0) {
        io_error("dap_listen");
        return false;
    }

    alert(DAP_PREFIX "Listening for connection on port %u.", dap_port);

    struct sockaddr_in sa_dap;
    socklen_t address_len = sizeof(sa_dap);
    connection_fd = accept(sock, (struct sockaddr *) &sa_dap, &address_len);
    if (connection_fd < 0) {
        if (errno == EINTR) {
            alert(DAP_PREFIX "Interrupted");
        } else {
            io_error("dap_accept");
        }

        return false;
    }

    alert(DAP_PREFIX "Connected.");
    return true;
}

/* MSIM DAP protocol implementation */

/** Receive bytes from DAP connection
 *
 * This will try to receive `INBOUND_FRAME_SIZE` bytes and store
 * them in `buf`.
 * If not enough bytes are available, it will return false and
 * buffer the received bytes internally until enough bytes are
 * available. The buffer provided is not modified in this case.
 * @param buf Pointer to buffer for received data. Only modified if the function returns true.
 * @param block Whether to block until bytes are available.
 * If false, the function will return immediately if no bytes are available, without buffering.
 * @return True if successfully received `INBOUND_FRAME_SIZE` bytes and stored them in `buf`.
 * False otherwise.
 */
static bool dap_receive_bytes(void *buf, const bool block)
{
    const ssize_t need_bytes = INBOUND_FRAME_SIZE - frame_buffered_bytes;
    uint8_t *write_buffer = frame_buffer + frame_buffered_bytes;
    ASSERT(need_bytes <= INBOUND_FRAME_SIZE);
    ASSERT(0 < need_bytes);
    ASSERT(connection_fd != -1);
    ASSERT(buf != NULL);
    ASSERT(write_buffer + need_bytes <= frame_buffer + INBOUND_FRAME_SIZE); // No overflow check

    const int flags = block ? MSG_WAITALL : MSG_DONTWAIT;
    const ssize_t received = recv(connection_fd, write_buffer, need_bytes, flags);

    // Received
    if (received > 0) {
        frame_buffered_bytes += received;

        // Got enough
        if (received == need_bytes) {
            ASSERT(frame_buffered_bytes == INBOUND_FRAME_SIZE);
            memcpy(buf, frame_buffer, INBOUND_FRAME_SIZE);
            frame_buffered_bytes = 0;
            return true;
        }

        // Not enough, buffer
        ASSERT(received < need_bytes);
        ASSERT(frame_buffered_bytes < INBOUND_FRAME_SIZE);
        return false;
    }

    // No bytes for now
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        return false;
    }

    // Closed or had errors, exit

    if (received < 0) {
        io_error("dap_recv");
    }
    dap_state = DAP_DONE;
    return false;
}

/** Send bytes to DAP connection, blocking until all successfully sent
 *
 * This will try to send `OUTBOUND_FRAME_SIZE` bytes from `buf`.
 * If not all bytes can be sent at once, it will block and retry
 * until all bytes are sent.
 * The buffer provided is not modified until the operation is
 * successful.
 * @param buf Pointer to buffer with data to send.
 * @return True if successful.
 */
static bool dap_send_bytes(const void *buf)
{
    ssize_t to_send = OUTBOUND_FRAME_SIZE;
    const uint8_t *ptr = buf;
    while (to_send > 0) {
        const ssize_t sent = send(connection_fd, ptr, to_send, 0);
        // Error
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            io_error("dap_send");
            dap_state = DAP_DONE;
            return false;
        }
        // Connection closed
        if (sent == 0) {
            dap_state = DAP_DONE;
            return false;
        }
        ptr += sent;
        to_send -= sent;
    }
    return true;
}

/** Receive a DAP request
 *
 * @param out_cmd Pointer to output request structure. Only modified if the function returns true.
 * @param block Whether to block until a full request is available.
 * @return True if successfully received a full request and stored it in `out_cmd`. False otherwise.
 */
static bool dap_receive_request(dap_request_t *out_cmd, const bool block)
{
    uint8_t buffer[INBOUND_FRAME_SIZE] = { 0 };
    if (!dap_receive_bytes(buffer, block)) {
        return false;
    }

    out_cmd->type = buffer[0];

    uint64_t arg0_no;
    uint64_t arg1_no;
    uint64_t arg2_no;
    memcpy(&arg0_no, buffer + sizeof(uint8_t) + 0 * sizeof(uint64_t), sizeof(arg0_no));
    memcpy(&arg1_no, buffer + sizeof(uint8_t) + 1 * sizeof(uint64_t), sizeof(arg1_no));
    memcpy(&arg2_no, buffer + sizeof(uint8_t) + 2 * sizeof(uint64_t), sizeof(arg2_no));

    out_cmd->arg0 = be64toh(arg0_no);
    out_cmd->arg1 = be64toh(arg1_no);
    out_cmd->arg2 = be64toh(arg2_no);

    return true;
}

/** Send a DAP response, blocking until successful
 *
 * @param response Response to send.
 * @return True if successful.
 */
static bool dap_send_response(const dap_response_t response)
{
    uint8_t buffer[OUTBOUND_FRAME_SIZE] = { 0 };
    buffer[0] = ResponseCategory;
    buffer[1] = (uint8_t) response.type;

    const uint64_t arg0_no = htobe64(response.arg0);
    const uint64_t arg1_no = htobe64(response.arg1);
    const uint64_t arg2_no = htobe64(response.arg2);
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 0 * sizeof(uint64_t), &arg0_no, sizeof(arg0_no));
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 1 * sizeof(uint64_t), &arg1_no, sizeof(arg1_no));
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 2 * sizeof(uint64_t), &arg2_no, sizeof(arg2_no));

    return dap_send_bytes(buffer);
}

/** Send a DAP event, blocking until successful
 *
 * @param event Event to send.
 * @return True if successful.
 */
static bool dap_send_event(const dap_event_t event)
{
    uint8_t buffer[OUTBOUND_FRAME_SIZE] = { 0 };
    buffer[0] = EventCategory;
    buffer[1] = (uint8_t) event.type;

    const uint64_t arg0_no = htobe64(event.arg0);
    const uint64_t arg1_no = htobe64(event.arg1);
    const uint64_t arg2_no = htobe64(event.arg2);
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 0 * sizeof(uint64_t), &arg0_no, sizeof(arg0_no));
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 1 * sizeof(uint64_t), &arg1_no, sizeof(arg1_no));
    memcpy(buffer + sizeof(uint8_t) + sizeof(uint8_t) + 2 * sizeof(uint64_t), &arg2_no, sizeof(arg2_no));

    return dap_send_bytes(buffer);
}

/* DAP state transitions */

void dap_close(void)
{
    if (connection_fd != -1) {
        alert(DAP_PREFIX "Sending terminated event.");
        dap_send_event((dap_event_t) { TerminatedEvent, 0x00, 0x00, 0x00 });

        if (close(connection_fd) == -1) {
            io_error("dap_connection_fd");
        }
        connection_fd = -1;
    }

    dap_state = DAP_DONE;
    machine_halt = true;
    alert(DAP_PREFIX "Connection closed.");
}

/* Generic DAP helpers */

// Get the CPU with the given ID, or respond with an error if no such CPU exists. Returns NULL in this case.
static general_cpu_t *get_cpu_or_respond_error(const uint64_t cpu_id)
{
    general_cpu_t *cpu = get_cpu(cpu_id);
    if (cpu == NULL) {
        alert(DAP_PREFIX "No such CPU with ID %" PRIu64 "!", cpu_id);
        dap_send_response((dap_response_t) { StatusCpuNotFoundError, cpu_id, 0x00, 0x00 });
    }
    return cpu;
}

static bool dap_validate_phys_addr_or_respond_error(const uint64_t address, ptr36_t *out_phys_addr)
{
    // Physical addresses are at most 36 bits
    if (address & ~(uint64_t) 0xfffffffff) {
        dap_send_response((dap_response_t) { StatusBadAddressError, address, 0x00, 0x00 });
        return false;
    }
    *out_phys_addr = (ptr36_t) address;
    return true;
}

static bool dap_translate_virt_or_respond_error(const uint64_t cpu_id, const uint64_t virt_addr, ptr36_t *out_phys_addr)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return false;
    }

    const ptr64_t virt = { .ptr = virt_addr };
    if (!cpu_convert_addr(cpu, virt, out_phys_addr, false)) {
        alert(DAP_PREFIX "Failed to translate virtual address %#0" PRIx64 "!", virt_addr);
        dap_send_response((dap_response_t) { StatusBadAddressError, virt_addr, 0x00, 0x00 });
        return false;
    }

    return true;
}

/* Simulator events */

void dap_event_hit_code_breakpoint(const unsigned int cpu_no)
{
    const uint64_t address = cpu_get_pc(get_cpu(cpu_no)).ptr;
    alert(DAP_PREFIX "Hit code breakpoint at address %#0" PRIx64 ", stopping", address);
    dap_state = DAP_PAUSED; // Can't hit BP while paused, so we must have been running
    dap_send_event((dap_event_t) { StoppedAtEvent, cpu_no, address, StoppedReasonBreakpoint });
}

void dap_event_hit_data_breakpoint(const uint64_t address)
{
    alert(DAP_PREFIX "Hit data breakpoint at address %#0" PRIx64 ", stopping", address);
    dap_state = DAP_PAUSED; // Can't hit BP while paused, so we must have been running
    // No way to know which CPU caused the data breakpoint, so we use the default one
    dap_send_event((dap_event_t) { StoppedAtEvent, cpuno_default, address, StoppedReasonBreakpoint });
}

/* Handlers */

static void dap_handle_resume(void)
{
    dap_state = DAP_RUNNING;
    alert(DAP_PREFIX "Resuming execution.");
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_pause(void)
{
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });

    // Avoid duplicate events
    if (dap_state == DAP_PAUSED) {
        return;
    }

    dap_send_event((dap_event_t) { StoppedAtEvent,
            cpuno_default, cpu_get_pc(get_cpu(cpuno_default)).ptr, StoppedReasonPaused });

    alert(DAP_PREFIX "Pausing execution.");
    dap_state = DAP_PAUSED;
}

static void dap_handle_terminate(void)
{
    alert(DAP_PREFIX "Got terminate request, exiting.");
    // Exited event is handled in dap_close(), which is always called at the end.
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
    dap_state = DAP_DONE;
}

/** Handle set code breakpoint request */
static void dap_handle_set_code_breakpoint(const uint64_t addr)
{
    const ptr64_t virt_addr = { .ptr = addr };

    general_cpu_t *cpu = NULL;
    uint32_t set_cpus = 0;
    for_each(cpu_list, cpu, general_cpu_t)
    {
        set_cpus += cpu_insert_breakpoint(cpu, virt_addr, BREAKPOINT_KIND_DEBUGGER) ? 1 : 0;
    }

    if (set_cpus > 0) {
        alert(DAP_PREFIX "Added code breakpoint at address %#0" PRIx64, virt_addr.ptr);
        dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
    } else {
        alert(DAP_PREFIX "Error setting breakpoint at address %#0" PRIx64 ": no such address in any CPU!", virt_addr.ptr);
        dap_send_response((dap_response_t) { StatusBadAddressError, addr, 0x00, 0x00 });
    }
}

/** Handle remove code breakpoint request */
static void dap_handle_remove_code_breakpoint(const uint64_t addr)
{
    const ptr64_t virt_addr = { .ptr = addr };

    general_cpu_t *cpu = NULL;
    uint32_t removed_cpus = 0;
    for_each(cpu_list, cpu, general_cpu_t)
    {
        removed_cpus += cpu_remove_breakpoint(cpu, virt_addr, BREAKPOINT_KIND_DEBUGGER) ? 1 : 0;
    }

    if (removed_cpus > 0) {
        alert(DAP_PREFIX "Removed code breakpoint at address %#0" PRIx64, virt_addr.ptr);
        dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
    } else {
        alert(DAP_PREFIX "Error removing breakpoint at address %#0" PRIx64 ": no such address in any CPU!", virt_addr.ptr);
        dap_send_response((dap_response_t) { StatusBadAddressError, addr, 0x00, 0x00 });
    }
}

static void dap_handle_set_data_breakpoint(const uint64_t addr, const uint64_t kind, const uint64_t size)
{
    ptr36_t phys_addr = 0;
    if (!dap_validate_phys_addr_or_respond_error(addr, &phys_addr)) {
        return;
    }

    const access_filter_t bp_kind = kind;
    if (bp_kind != ACCESS_FILTER_READ && bp_kind != ACCESS_FILTER_WRITE && bp_kind != ACCESS_FILTER_ANY) {
        alert(DAP_PREFIX "Invalid data breakpoint kind %#0" PRIx64 "!", kind);
        dap_send_response((dap_response_t) { StatusUnspecifiedError, kind, 0x00, 0x00 });
        return;
    }

    physmem_breakpoint_add(phys_addr, size, BREAKPOINT_KIND_DEBUGGER, bp_kind);
    alert(DAP_PREFIX "Added data breakpoint at physical address %#0" PRIx64 "of type %" PRIu64 " and size %" PRIu64, phys_addr, kind, size);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_remove_data_breakpoint(const uint64_t addr)
{
    ptr36_t phys_addr = 0;
    if (!dap_validate_phys_addr_or_respond_error(addr, &phys_addr)) {
        return;
    }

    physmem_breakpoint_remove(phys_addr);
    alert(DAP_PREFIX "Removed data breakpoint at physical address %#0" PRIx64, phys_addr);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_step(const uint64_t cpu_id, const uint64_t count)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    cpu->steps_left = count;
    alert(DAP_PREFIX "Stepping %" PRIu64 " instructions on CPU %" PRIu64 ".", count, cpu_id);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_read_register(const uint64_t cpu_id, const uint64_t reg_id)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    uint64_t reg_value = 0;
    if (!cpu_get_reg(cpu, reg_id, &reg_value)) {
        alert(DAP_PREFIX "Failed to read general register ID %" PRIu64 "!", reg_id);
        dap_send_response((dap_response_t) { StatusRegisterNotFoundError, reg_id, 0x00, 0x00 });
        return;
    }

    dap_send_response((dap_response_t) { StatusOk, reg_value, 0x00, 0x00 });
}

static void dap_handle_write_register(const uint64_t cpu_id, const uint64_t reg_id, const uint64_t value)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    if (!cpu_set_reg(cpu, reg_id, value)) {
        alert(DAP_PREFIX "Failed to write general register ID %" PRIu64 "!", reg_id);
        dap_send_response((dap_response_t) { StatusRegisterNotFoundError, reg_id, 0x00, 0x00 });
        return;
    }

    alert(DAP_PREFIX "Received WriteGeneralRegisterRequest for register ID %" PRIu64 " with value %#0" PRIx64, reg_id, value);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_read_csr(const uint64_t cpu_id, const uint64_t reg_id)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    uint64_t reg_value = 0;
    if (!cpu_get_csr(cpu, reg_id, &reg_value)) {
        alert(DAP_PREFIX "Failed to read CSR %#0" PRIx64 "!", reg_id);
        dap_send_response((dap_response_t) { StatusRegisterNotFoundError, reg_id, 0x00, 0x00 });
        return;
    }

    dap_send_response((dap_response_t) { StatusOk, reg_value, 0x00, 0x00 });
}

static void dap_handle_write_csr(const uint64_t cpu_id, const uint64_t reg_id, const uint64_t value)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    if (!cpu_set_csr(cpu, reg_id, value)) {
        alert(DAP_PREFIX "Failed to write CSR %#0" PRIx64 "!", reg_id);
        dap_send_response((dap_response_t) { StatusRegisterNotFoundError, reg_id, 0x00, 0x00 });
        return;
    }

    alert(DAP_PREFIX "Received WriteGeneralRegisterRequest for register ID %" PRIu64 " with value %#0" PRIx64, reg_id, value);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_read_pc(const uint64_t cpu_id)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    const ptr64_t pc = cpu_get_pc(cpu);
    dap_send_response((dap_response_t) { StatusOk, pc.ptr, 0x00, 0x00 });
}

static void dap_handle_write_pc(const uint64_t cpu_id, const uint64_t value)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    const ptr64_t pc = { value };
    cpu_set_pc(cpu, pc);

    alert(DAP_PREFIX "Received WritePCRequest with value %#0" PRIx64, value);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_read_phys_memory(const uint64_t address)
{
    ptr36_t phys_addr = 0;
    if (!dap_validate_phys_addr_or_respond_error(address, &phys_addr)) {
        return;
    }

    // Read by uint8 to not have to worry about alignment
    uint8_t buffer[DAP_ARG_COUNT * sizeof(uint64_t)] = { 0 };
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        buffer[i] = physmem_read8(-1, phys_addr + i, false);
    }

    dap_response_t response = { .type = StatusOk };
    memcpy(&response.arg0, buffer + 0 * sizeof(uint64_t), sizeof(response.arg0));
    memcpy(&response.arg1, buffer + 1 * sizeof(uint64_t), sizeof(response.arg1));
    memcpy(&response.arg2, buffer + 2 * sizeof(uint64_t), sizeof(response.arg2));

    dap_send_response(response);
}

static void dap_handle_write_phys_memory(const uint64_t address, const uint64_t value)
{
    ptr36_t phys_addr = 0;
    if (!dap_validate_phys_addr_or_respond_error(address, &phys_addr)) {
        return;
    }

    // Write by uint8 to not have to worry about alignment
    uint8_t buffer[sizeof(uint64_t)] = { 0 };
    memcpy(buffer, &value, sizeof(value));
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        const bool success = physmem_write8(-1, phys_addr + i, buffer[i], false);
        if (!success) {
            alert(DAP_PREFIX "Failed to write to physical address %#0" PRIx64 "!", phys_addr + i);
            dap_send_response((dap_response_t) { StatusBadAddressError, address, 0x00, 0x00 });
            return;
        }
    }

    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_read_virt_memory(const uint64_t cpu_id, const uint64_t address)
{
    ptr36_t phys_addr = 0;
    dap_translate_virt_or_respond_error(cpu_id, address, &phys_addr);
    dap_handle_read_phys_memory(phys_addr);
}

static void dap_handle_write_virt_memory(const uint64_t cpu_id, const uint64_t address, const uint64_t value)
{
    ptr36_t phys_addr = 0;
    dap_translate_virt_or_respond_error(cpu_id, address, &phys_addr);
    dap_handle_write_phys_memory(phys_addr, value);
}

static void dap_handle_translate_address(const uint64_t cpu_id, const uint64_t address)
{
    ptr36_t phys_addr = 0;
    if (dap_translate_virt_or_respond_error(cpu_id, address, &phys_addr)) {
        dap_send_response((dap_response_t) { StatusOk, phys_addr, 0x00, 0x00 });
    }
}

static void dap_handle_raise_interrupt(const uint64_t cpu_id, const uint64_t interrupt_id)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    cpu_interrupt_up(cpu, interrupt_id);
    alert(DAP_PREFIX "Raised interrupt ID %" PRIu64 " on CPU %" PRIu64 ".", interrupt_id, cpu_id);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_clear_interrupt(const uint64_t cpu_id, const uint64_t interrupt_id)
{
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    cpu_interrupt_down(cpu, interrupt_id);
    alert(DAP_PREFIX "Cleared interrupt ID %" PRIu64 " on CPU %" PRIu64 ".", interrupt_id, cpu_id);
    dap_send_response((dap_response_t) { StatusOk, 0x00, 0x00, 0x00 });
}

static void dap_handle_get_config(void)
{
    uint64_t cpu_count = 0;
    general_cpu_t *cpu;
    for_each(cpu_list, cpu, general_cpu_t)
    {
        ++cpu_count;
    }

    alert(DAP_PREFIX "Received GetConfigRequest, reporting %" PRIu64 " CPUs.", cpu_count);
    dap_send_response((dap_response_t) { StatusOk, cpu_count, 0x00, 0x00 });
}

static void dap_handle_get_cpu_info(const uint64_t cpu_id)
{
    alert(DAP_PREFIX "Received GetCpuInfoRequest for CPU ID %" PRIu64 "", cpu_id);
    general_cpu_t *cpu = get_cpu_or_respond_error(cpu_id);
    if (cpu == NULL) {
        return;
    }

    uint64_t arch_val = 0x00;
    switch (cpu_get_arch(cpu)) {
    case CpuArchMips:
        arch_val = 0x01;
        break;
    case CpuArchRiscV32:
        arch_val = 0x02;
        break;
    case CpuArchRiscV64:
        arch_val = 0x03;
        break;
    default:
        alert(DAP_PREFIX "CPU with ID %" PRIu64 " has unknown architecture %u!", cpu_id, cpu->type->arch);
        arch_val = 0xFF;
    }

    dap_send_response((dap_response_t) { StatusOk, arch_val, 0x00, 0x00 });
}

static void dap_check_step(void)
{
    general_cpu_t *cpu = NULL;
    for_each(cpu_list, cpu, general_cpu_t)
    {
        if (cpu->steps_left > 0 && --cpu->steps_left == 0) {
            alert(DAP_PREFIX "Finished stepping on CPU %u", cpu->cpuno);
            dap_send_event((dap_event_t) { StoppedAtEvent,
                    cpu->cpuno, cpu_get_pc(cpu).ptr, StoppedReasonStep });
            dap_state = DAP_PAUSED;
        }
    }
}

void dap_process(void)
{
    dap_check_step();

    dap_request_t request = { 0 };

    while (dap_receive_request(&request, dap_state == DAP_PAUSED)) {
        switch (request.type) {
        case ResumeRequest:
            dap_handle_resume();
            continue;
        case PauseRequest:
            dap_handle_pause();
            continue;
        case TerminateRequest:
            dap_handle_terminate();
            return;
        case StepRequest:
            dap_handle_step(request.arg0, request.arg1);
            continue;
        case SetCodeBreakpointRequest:
            dap_handle_set_code_breakpoint(request.arg0);
            continue;
        case RemoveCodeBreakpointRequest:
            dap_handle_remove_code_breakpoint(request.arg0);
            continue;
        case SetDataBreakpointRequest:
            dap_handle_set_data_breakpoint(request.arg0, request.arg1, request.arg2);
            continue;
        case RemoveDataBreakpointRequest:
            dap_handle_remove_data_breakpoint(request.arg0);
            continue;
        case ReadGeneralRegisterRequest:
            dap_handle_read_register(request.arg0, request.arg1);
            continue;
        case WriteGeneralRegisterRequest:
            dap_handle_write_register(request.arg0, request.arg1, request.arg2);
            continue;
        case ReadCsrRequest:
            dap_handle_read_csr(request.arg0, request.arg1);
            continue;
        case WriteCsrRequest:
            dap_handle_write_csr(request.arg0, request.arg1, request.arg2);
            continue;
        case ReadPCRequest:
            dap_handle_read_pc(request.arg0);
            continue;
        case WritePCRequest:
            dap_handle_write_pc(request.arg0, request.arg1);
            continue;
        case ReadPhysMemoryRequest:
            dap_handle_read_phys_memory(request.arg0);
            continue;
        case WritePhysMemoryRequest:
            dap_handle_write_phys_memory(request.arg0, request.arg1);
            continue;
        case ReadVirtMemoryRequest:
            dap_handle_read_virt_memory(request.arg0, request.arg1);
            continue;
        case WriteVirtMemoryRequest:
            dap_handle_write_virt_memory(request.arg0, request.arg1, request.arg2);
            continue;
        case TranslateAddressRequest:
            dap_handle_translate_address(request.arg0, request.arg1);
            continue;
        case RaiseInterruptRequest:
            dap_handle_raise_interrupt(request.arg0, request.arg1);
            continue;
        case ClearInterruptRequest:
            dap_handle_clear_interrupt(request.arg0, request.arg1);
            continue;
        case GetConfigRequest:
            dap_handle_get_config();
            continue;
        case GetCpuInfoRequest:
            dap_handle_get_cpu_info(request.arg0);
            continue;

        default:
            alert(DAP_PREFIX "Unknown request type %u.", request.type);
            dap_send_response((dap_response_t) { StatusUnsupportedRequestError, request.type, 0x00, 0x00 });
        }
    }
}
