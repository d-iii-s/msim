#define ehalt .word 0x8C000073
.text

/** Multiple tests testing the TLB
 *  All of the tests use two different pagetables that translate the same virtual address to a different physical address
 *  
 *  1. Both translations have the same ASID with no SFENCE between the accesses
 *  2. Both translations have the same ASID with full SFENCE between the accesses
 *  3. Both translations have the same ASID with address SFENCE between the accesses
 *  4. Both translations have the same ASID with asid SFENCE between the accesses
 *  5. Both translations have the same ASID with asid and address SFENCE between the accesses
 *  6. The translations have a different ASID with no SFENCE between the accesses
 */








// Sets up the translation for the first pagetable
// The root is expected to be located in register a0
setup_first_pagetable:

    // Setup execute only global megapage for main execution instructions
    // The megapage is located at 0xF0000000
    li t1, 0xF00
    add t2, a0, t1
    li t1, 0x3C000029
    sw t1, (t2)

    // Setup address 0x0 to be mapped to 0x0 via a megapage
    li t1, 0x0000000F
    sw t1, (a0)



success:
    li t0, 0x90000000
    li t1, 'S'
    sw t1, (t0)
    ehalt

fail:
    li t0, 0x90000000
    li t1, 'F'
    sw t1, (t0)
    ehalt