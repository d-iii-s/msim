#define ehalt .word 0x00200073
user_read_user_page:    // 
    li t0, 0x10003000

    // no trap
    lw zero, (t0)

jal ra, newline

user_read_supervisor_page: // T
    li t0, 0x10000000

    // trap
    lw zero, (t0)

jal ra, newline

// End of all tests
ehalt

newline: 
    li t0, 0xA0000000
    li t1, '\n'
    sw t1, (t0)
    ret
