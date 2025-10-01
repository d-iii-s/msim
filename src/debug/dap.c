#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../device/cpu/general_cpu.h"
#include "../device/cpu/mips_r4000/cpu.h"
#include "../fault.h"
#include "../main.h"
#include "dap.h"

static int dap_connection_fd = -1;
static unsigned int cpuno_global = 0; // TODO: what is it?

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

    alert("DAP: Waiting for connection on port %u", dap_port);

    struct sockaddr_in sa_dap;
    socklen_t address_len = sizeof(sa_dap);
    dap_connection_fd = accept(sock, (struct sockaddr *) &sa_dap, &address_len);
    if (dap_connection_fd < 0) {
        if (errno == EINTR) {
            alert("DAP: Interrupted");
        } else {
            io_error("dap_accept");
        }

        return false;
    }

    alert("DAP: Connected");
    return true;
}

static void dap_close(void)
{
    if (dap_connection_fd == -1) {
        io_error("dap_already_closed");
    }

    if (close(dap_connection_fd) == -1) {
        io_error("dap_connection_fd");
    }

    dap_connection_fd = -1;
    dap_connected = false;
}

static bool dap_receive(dap_command_t *out_cmd)
{
    dap_command_t command = { 0 };

    if (recv(dap_connection_fd, &command.type, 1, 0) <= 0) {
        // TODO: dap_close(); ?
        return false;
    }

    if (recv(dap_connection_fd, &command.addr, 4, 0) <= 0) {
        return false;
    }

    *out_cmd = command;
    return true;
}

/** Add a DAP breakpoint */
static void dap_breakpoint_add(const uint32_t addr)
{
    alert("Adding DAP breakpoint at address 0x%u.", addr);

    ptr64_t virt_address;
    virt_address.ptr = UINT64_C(0xffffffff00000000) | addr;
    r4k_cpu_t *cpu = get_cpu(cpuno_global)->data;

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
    alert("Removing DAP breakpoint from address 0x%u.", addr);

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
    if (!dap_receive(&command)) {
        alert("DAP connection lost.");
        dap_close();
        return;
    }

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
