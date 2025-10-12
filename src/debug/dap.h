#ifndef MSIM_DAP_H
#define MSIM_DAP_H

/** Initialize DAP connection
 *
 */
extern bool dap_init(void);

/** Process new DAP events
 * (main DAP callback)
 */
extern void dap_process(void);

extern void dap_close(void);

#endif // MSIM_DAP_H
