/* Virtual addresses shared between kernel.riscv32.c (which sets up the
 * page tables) and app.riscv32.S (the U-mode program), since the two are
 * compiled and linked completely independently and so cannot share C
 * symbols. */

#define APP_ADDRESS               0x10008000u
#define SUPERVISOR_ONLY_PAGE_ADDR 0x10000000u
#define USER_ACCESSIBLE_PAGE_ADDR 0x10003000u
