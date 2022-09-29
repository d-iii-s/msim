#define ehalt .word 0x00200073
#define scyclecmp 0x5C0
#define mstatus_mie 1<<3
#define sti 1<<5
li t0, 100
csrw scyclecmp, t0
csrsi mstatus, mstatus_mie
li t0, sti
csrs mie, t0
csrwi mcycle, 0
wfi
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
ehalt