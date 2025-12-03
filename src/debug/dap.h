#ifndef MSIM_DAP_H
#define MSIM_DAP_H

typedef enum {
    DAP_READY, // DAP initialized, waiting for connection
    DAP_CONNECTED, // DAP connected, ready to process requests
    DAP_RUNNING, // DAP running
    DAP_DONE // DAP session done
} dap_state_t;

/** Initialize DAP connection
 *
 */
extern bool dap_init(void);

/** Process new DAP requests
 * (main DAP callback)
 *
 * @return Processing decision
 */
extern void dap_process(void);

extern void dap_close(void);

#endif // MSIM_DAP_H
