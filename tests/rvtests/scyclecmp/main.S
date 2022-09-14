#define scyclecmp 0x5C0
#define mstatus_mie 1<<3
#define sti 1<<5
ebreak
li t0, 100
csrw scyclecmp, t0
csrsi mstatus, mstatus_mie
li t0, sti
csrs mie, t0
csrwi mcycle, 0
cycle:
    addi a0, a0, 1
    j cycle