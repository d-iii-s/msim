#include <pcut/pcut.h>

#include "../../../src/main.h"

/*
 * MSIM main.cpp MOCK
 */

/** Configuration file name */
char *config_file = NULL;

/** Remote GDB debugging */
bool remote_gdb = false;
unsigned int remote_gdb_port = 0;
bool remote_gdb_conn = false;
bool remote_gdb_listen = false;
bool remote_gdb_step = false;

// DAP debugging
bool dap_enabled = false;
unsigned int dap_port = 10505;
dap_state_t dap_state = DAP_READY;

/** General simulator behaviour */
bool machine_nondet = false;

// set to true for debugging
bool machine_trace = false;
bool machine_halt = false;
bool machine_break = false;
bool machine_interactive = false;
bool machine_newline = false;
bool machine_undefined = false;
bool machine_specific_instructions = true;
bool machine_allow_interactive_without_tty = false;
uint64_t stepping = 0;

PCUT_INIT

PCUT_IMPORT(instruction_immediates);
PCUT_IMPORT(instruction_decoding);
PCUT_IMPORT(instruction_exceptions);
PCUT_IMPORT(tlb);
PCUT_IMPORT(asid_len);

PCUT_MAIN()
