#define ehalt .word 0x00200073
.text
li t0, 0x80000000
csrw mtvec, t0
jal ra, setup_table
li t0, 0x800FFFD0
csrw satp, t0
li t0, 1 << 11
csrs mstatus, t0
auipc t0, 0
addi t0, t0, 16
csrw mepc, t0
mret
lw t0, (zero)
ehalt

// Sets up the root of the address translation tree
// The root is expected to be located at 0xFFFD0000
setup_table:
li t0, 0xFFFD0000

// Setup megapage for main execution instructions
// The megapage is located at 0xFF000000
li t1, 0xF00
add t2, t0, t1
li t1, 0x3C00000B
sw t1, (t2)

ret