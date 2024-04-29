#define ehalt .word 0x8C000073
#define mstatus_mpp_s (0b01<<11)
#define mstatus_mprv (1<<17)
.text
// tests that instruction fetches are not translated even when mstatus.MPRV is set

// Setup a failing exception handler
li t0, 0x70000000
csrw mtvec, t0

// load satp with some bogus value that should result in an page fault if the instruction would be fetched from it
li t0, 0x80000000
csrw satp, t0

// set mstatus to use S-level memory translation
li t0, mstatus_mpp_s | mstatus_mprv
csrs mstatus, t0

// Perform an instruction, should pagefault
addi t0, t1, 0

// set mstatus back to no translation
li t0, mstatus_mprv
csrc mstatus, t0

// Print success and halt
li t0, 0x90000000
li t1, 'S'
sw t1, (t0)
ehalt
