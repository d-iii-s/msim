#define ehalt .word 0x8C000073
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
// Enable machine interrupts, SSI should be masked and not trap
csrsi mstatus, 8
// Print C to know that we have not trapped
li t0, 0x90000000
li t1, 'C'
sw t1, (t0)
// Disable delegation, SSI should be raised, and we should trap to M
csrci mideleg, 2
// Fail
li t0, 0x90000000
li t1, 'F'
sw t1, (t0)
ehalt
