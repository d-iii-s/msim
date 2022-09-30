#define ehalt .word 0x00200073
.text
li t0, 0x80000000
csrw stvec, t0
li t0, 0x70000000
csrw mtvec, t0
// Enable SSI
csrwi mie, 2
// Set SSI pending
csrsi mip, 2
// Delegate SSI
csrsi mideleg, 2
// Enable supervisor interrupts
csrsi mstatus, 2
// Set MPP as S mode
li t0, 1 << 11
csrs mstatus, t0
// Setup mret address
auipc t0, 0
addi t0, t0, 16
csrw mepc, t0 
// mret to S mode, should raise interrupt
mret
// Fail
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
ehalt