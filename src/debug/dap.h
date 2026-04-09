#ifndef MSIM_DAP_H
#define MSIM_DAP_H

typedef enum {
    DAP_INIT, // DAP not connected yet
    DAP_RUNNING, // Running the simulation
    DAP_PAUSED, // Paused, waiting for requests
    DAP_DONE // DAP session done
} dap_state_t;

/** Initialize DAP connection
 *
 */
extern bool dap_init(void);

/** Process new DAP requests
 * (main DAP callback)
 * Blocks only when dap_state is DAP_PAUSED, otherwise it will
 * return immediately if there are no requests to process.
 */
extern void dap_process(void);

/** Close DAP connection and cleanup
 *
 * This will also send an "exited" event to the DAP client, so it should be called when the simulation is exiting.
 */
extern void dap_close(void);

extern void dap_event_hit_code_breakpoint(uint64_t address);

#endif // MSIM_DAP_H
