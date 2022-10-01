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
jal ra, access_dirty_test
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

    // Setup printer translation
    li t1, 0x900
    add t2, t0, t1
    li t1, 0x2400000B
    sw t1, (t2)

    // Setup translation megapage for the vtable entries
    li t1, 0xFFC
    add t2, t0, t1
    li t1, 0x3FF0000B
    sw t1, (t2)

    // Setup first level pte for tests
    li t1, 0x100
    add t2, t0, t1
    li t1, 0x3FFF8001
    sw t1, (t2)

    // Now setup translations for the tests
    li t0, 0xFFFE0000

    // Setup rwx page
    li t1, 0x000
    add t2, t0, t1
    li t1, 0x0400000F
    sw t1, (t2)
    ret


access_dirty_test:
    li t0, 0xFFFE0000
    li t1, 0x10000000

    lw t2, (t0)
    andi t2, t2, 0xC0
    bnez t2, fail
    
    // read from rwx page
    lw t2, (t1)

    lw t2, (t0)
    andi t2, t2, 0xC0
    addi t2, t2, - (1<<6)    
    bnez t2, fail

    // write to rwx page
    ebreak
    sw t2, (t1)

    lw t2, (t0)
    andi t2, t2, 0xC0
    addi t2, t2, - (3<<6)    
    bnez t2, fail

    ret

fail:
    li t0, 0x90000000
    li t1, 'F'
    sw t1, (t0)
    ehalt


