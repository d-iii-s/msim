#define ehalt .word 0x8C000073
#define mstatus_mie 1<<3
#define sti 1<<5

// Test that setting STIP from M mode triggers an interrupt

li t0, sti
csrs mie, t0
csrs mip, t0
csrsi mstatus, mstatus_mie // This should trap

li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
ehalt