#include "addresses.h"

.text
app_start:
    li t0, USER_ACCESSIBLE_PAGE_ADDR
    lw zero, (t0)              // U-mode, U-accessible page: must not trap

    li t0, 0x90000000
    li t1, '\n'
    sw t1, 0(t0)

    li t0, SUPERVISOR_ONLY_PAGE_ADDR
    lw zero, (t0)              // U-mode, S-only page: must trap

    li t0, 0x90000000
    li t1, '\n'
    sw t1, 0(t0)

    simulator_halt
