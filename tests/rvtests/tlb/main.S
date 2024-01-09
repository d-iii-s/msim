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

// Setup trap handling
li t0, 0x80000000
csrw mtvec, t0

// Setup the two pagetables
li a0, 0xFFFD0000
li a1, 0x0
jal ra, setup_pagetable
li a0, 0xFFFE0000
li a1, 0x00400000
jal ra, setup_pagetable

// Setup the data
li t0, 0x0
li t1, 1
sw t1, (t0)

li t0, 0x00400000
li t1, 2
sw t1, (t0)

// MPP = S
li t0, 1 << 11
csrw mstatus, t0

// Enter S mode
auipc t0, 0
addi t0, t0, 16
csrw mepc, t0
mret

// Test 1
jal ra, same_asid_no_sfence
// Test 2
jal ra, same_asid_full_sfence
// Here we have tested that the full flush works
// We will use it between tests to reset it
sfence.vma

// Test 3
jal ra, same_asid_correct_address_sfence
sfence.vma
jal ra, same_asid_incorrect_address_sfence
sfence.vma

// Test 4
jal ra, same_asid_correct_asid_sfence
sfence.vma
jal ra, same_asid_incorrect_asid_sfence
sfence.vma

// Test 5
jal ra, same_asid_correct_asid_and_addr_sfence
sfence.vma
jal ra, same_asid_correct_asid_and_incorrect_addr_sfence
sfence.vma
jal ra, same_asid_incorrect_asid_and_correct_addr_sfence
sfence.vma
jal ra, same_asid_incorrect_asid_and_addr_sfence
sfence.vma

// Test 6
jal ra, different_asid_no_sfence

j success

// Sets up the translation for the first pagetable
// The root is expected to be located in register a0
setup_pagetable:

    // Setup execute only global megapage for main execution instructions
    // The megapage is located at 0xF0000000
    li t1, 0xF00
    add t2, a0, t1
    li t1, 0x3C000029
    sw t1, (t2)

    // Setup printer translation
    li t1, 0x900
    add t2, a0, t1
    li t1, 0x24000007
    sw t1, (t2)

    // Setup address 0x400000 to be mapped to the address in a1 via a megapage
    // we use address 0x400000 for it not to fall into the same slot in the TLB as the code megapage

    // Shift the address right by two bits to align it with PTE structure
    srli t0, a1, 2

    // Or-in XWRV flags
    ori t1, t0, 0x0000000F
    
    // Store at offset 1
    sw t1, 4(a0)

    ret

same_asid_no_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // No SFENCE

    // Load from address 0x40000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is again 1
    li t0, 1
    bne a0, t0, fail

    ret

same_asid_full_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // Full SFENCE
    sfence.vma

    // Load from address 0x40000 again to get the right translation
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is now 2
    li t0, 2
    bne a0, t0, fail

    ret

same_asid_correct_address_sfence:

    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // Address SFENCE
    li t0, 0x400000
    sfence.vma t0, zero

    // Load from address 0x40000 to get the correct value
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is now 2
    li t0, 2
    bne a0, t0, fail

    ret

same_asid_incorrect_address_sfence:

    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // Address SFENCE on the wrong megapage
    li t0, 0x0
    sfence.vma t0, zero

    // Load from address 0x400000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1, because we have flushed the wrong address
    li t0, 1
    bne a0, t0, fail

    ret

same_asid_correct_asid_sfence:

    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID SFENCE on the right asid
    li t0, 0
    sfence.vma zero, t0

    // Load from address 0x400000 get the right translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 2
    li t0, 2
    bne a0, t0, fail

    ret

same_asid_incorrect_asid_sfence:

    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID SFENCE on the wrong asid
    li t0, 1
    sfence.vma zero, t0

    // Load from address 0x400000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1, because we have flushed the wrong asid
    li t0, 1
    bne a0, t0, fail

    ret

same_asid_correct_asid_and_addr_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID and Address SFENCE on the correct asid and address
    li t0, 0x400000
    li t1, 0
    sfence.vma t0, t1

    // Load from address 0x400000 again to get the right translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 2, because we have flushed the right asid and address
    li t0, 2
    bne a0, t0, fail

    ret

same_asid_correct_asid_and_incorrect_addr_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID and Address SFENCE on the correct asid but incorrect address
    li t0, 0x0
    li t1, 0
    sfence.vma t0, t1

    // Load from address 0x400000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1, because we have flushed the wrong address
    li t0, 1
    bne a0, t0, fail

    ret

same_asid_incorrect_asid_and_correct_addr_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID and Address SFENCE on the incorrect asid but correct address
    li t0, 0x400000
    li t1, 1
    sfence.vma t0, t1

    // Load from address 0x400000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1, because we have flushed the wrong asid
    li t0, 1
    bne a0, t0, fail

    ret

same_asid_incorrect_asid_and_addr_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 0 also for the second translation with pagetable at 0xFFFE0000
    li t0, 0x800FFFE0
    csrw satp, t0

    // ASID and Address SFENCE on the incorrect asid and address
    li t0, 0x0
    li t1, 1
    sfence.vma t0, t1

    // Load from address 0x400000 again to hopefully get the wrong translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1, because we have flushed the wrong asid and address
    li t0, 1
    bne a0, t0, fail

    ret

different_asid_no_sfence:
    // Use ASID 0 for the first translation with pagetable at 0xFFFD0000
    li t0, 0x800FFFD0
    csrw satp, t0

    // Load from address 0x40000 to move the address into the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 1
    li t0, 1
    bne a0, t0, fail
    
    // Use ASID 1 for the second translation with pagetable at 0xFFFE0000
    li t0, 0x804FFFE0
    csrw satp, t0

    // No SFENCE

    // Load from address 0x40000 again to hopefully get the right translation from the TLB
    li t0, 0x400000
    lw a0, (t0)

    // Expected value is 2
    li t0, 2
    bne a0, t0, fail

    ret

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
