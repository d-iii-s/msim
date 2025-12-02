#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../assert.h"
#include "../device/cpu/general_cpu.h"
#include "../device/cpu/mips_r4000/cpu.h"
#include "../device/cpu/riscv_rv32ima/cpu.h"
#include "../fault.h"
#include "../main.h"
#include "dap.h"

static int connection_fd = -1;
static unsigned int cpuno_global = 0; // TODO: what is it?

/** Internal buffer for incoming messages */
static uint8_t message_buffer[64];
static const ssize_t message_buffer_len = sizeof(message_buffer);
static ssize_t message_buffer_used = 0;

typedef enum dap_command_type {
    NO_OP = 0,
    BREAKPOINT = 1
} dap_command_type_t;

typedef struct __attribute__((__packed__)) dap_command {
    uint8_t type;
    uint32_t addr;
} dap_command_t;

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

    alert("Listening for DAP connection on port %u.", dap_port);

    struct sockaddr_in sa_dap;
    socklen_t address_len = sizeof(sa_dap);
    connection_fd = accept(sock, (struct sockaddr *) &sa_dap, &address_len);
    if (connection_fd < 0) {
        if (errno == EINTR) {
            alert("DAP: Interrupted");
        } else {
            io_error("dap_accept");
        }

        return false;
    }

    alert("DAP connected.");
    return true;
}

void dap_close(void)
{
    if (connection_fd == -1) {
        io_error("dap_already_closed");
    }

    if (close(connection_fd) == -1) {
        io_error("dap_connection_fd");
    }

    connection_fd = -1;
    dap_connected = false;
    alert("DAP connection closed.");
}

/** Receive bytes from DAP connection, non-blocking
 *
 * This will try to receive `len` bytes and store them in `buf`.
 * If not enough bytes are available, it will return false and
 * buffer the received bytes internally until enough bytes are
 * available. The buffer provided is not touched in this case.
 * @param buf Pointer to buffer for received data.
 * @param len Total number of bytes to receive.
 * @return True if successful.
 */
static bool dap_receive_bytes(void *buf, const ssize_t len)
{
    const ssize_t need_bytes = len - message_buffer_used;
    uint8_t *write_buffer = message_buffer + message_buffer_used;
    ASSERT(need_bytes <= message_buffer_len);
    ASSERT(len <= message_buffer_len);
    ASSERT(need_bytes > 0);
    ASSERT(connection_fd != -1);
    ASSERT(buf != NULL);
    ASSERT(len > 0);

    const ssize_t received = recv(connection_fd, write_buffer, need_bytes, MSG_DONTWAIT);

    // Received
    if (received > 0) {
        message_buffer_used += received;

        // Got enough
        if (received == need_bytes) {
            ASSERT(message_buffer_used == len);
            memcpy(buf, message_buffer, len);
            message_buffer_used = 0;
            return true;
        }

        // Not enough, buffer
        ASSERT(received < need_bytes);
        ASSERT(message_buffer_used < len);
        ASSERT(message_buffer_used < message_buffer_len);
        return false;
    }

    // Closed
    if (received == 0) {
        dap_close(); // TODO: ?
        return false;
    }

    // No bytes for now
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return false;
    }

    // Error
    io_error("dap_recv");
    return false;
}

/** Receive a DAP command, non-blocking
 *
 * @param out_cmd Pointer to output command structure.
 * @return True if successful.
 */
static bool dap_receive_command(dap_command_t *out_cmd)
{
    uint8_t buffer[sizeof(dap_command_t)] = { 0 };
    if (!dap_receive_bytes(buffer, sizeof(dap_command_t))) {
        return false;
    }

    out_cmd->type = buffer[0];

    uint32_t netorder_addr;
    memcpy(&netorder_addr, buffer + sizeof(out_cmd->type), sizeof(netorder_addr));
    out_cmd->addr = ntohl(netorder_addr);

    return true;
}

/** Add a DAP breakpoint */
static void dap_breakpoint_add(const uint32_t addr)
{
    alert("Adding DAP breakpoint at address 0x%x.", addr);

    ptr64_t virt_address;
    virt_address.ptr = UINT64_C(0xffffffff00000000) | addr;
    rv_cpu_t *cpu = get_cpu(cpuno_global)->data;

    const breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps, virt_address, BREAKPOINT_FILTER_DEBUGGER);
    if (breakpoint != NULL) {
        return;
    }

    breakpoint_t *inserted_breakpoint = breakpoint_init(virt_address, BREAKPOINT_KIND_DEBUGGER);
    list_append(&cpu->bps, &inserted_breakpoint->item);
}

/** Remove a DAP breakpoint */
static void dap_breakpoint_remove(const uint32_t addr)
{
    alert("Removing DAP breakpoint from address 0x%x.", addr);

    ptr64_t virt_address;
    virt_address.ptr = UINT64_C(0xffffffff00000000) | addr;
    r4k_cpu_t *cpu = get_cpu(cpuno_global)->data;

    breakpoint_t *breakpoint = breakpoint_find_by_address(cpu->bps, virt_address, BREAKPOINT_FILTER_DEBUGGER);
    if (breakpoint != NULL) {
        return;
    }

    list_remove(&cpu->bps, &breakpoint->item);
    safe_free(breakpoint)
}

void dap_process(void)
{
    dap_command_t command = { 0 };

    while (dap_receive_command(&command)) {
        machine_interactive = true;

        switch (command.type) {
        case NO_OP:
            break;
        case BREAKPOINT:
            dap_breakpoint_add(command.addr);
            break;
        default:
            alert("Unknown DAP command type %u.", command.type);
            break;
        }
    }
}
