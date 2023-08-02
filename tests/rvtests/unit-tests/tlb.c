#include <stdint.h>
#include <stdio.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/tlb.h"

PCUT_INIT

PCUT_TEST_SUITE(tlb);

rv_tlb_t tlb;

PCUT_TEST_BEFORE {
    rv_tlb_init(&tlb, DEFAULT_RV_KTLB_SIZE, DEFAULT_RV_MTLB_SIZE);
}

PCUT_TEST_AFTER {
    rv_tlb_done(&tlb);
}

PCUT_TEST(simple){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    ptr36_t expected_phys = 0x0;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(get_with_offset){
    
    uint32_t virt = 0x0;
    ptr36_t phys  = 0x1000;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    ptr36_t mapped_phys = 0xFF;

    uint32_t requested_virt = 0x0001;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &mapped_phys);

    ptr36_t expected_phys = 0x1001;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(add_with_offset){
    
    uint32_t virt = 0x0001;
    ptr36_t phys  = 0x1000;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    ptr36_t mapped_phys = 0xFF;

    uint32_t requested_virt = 0x0;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &mapped_phys);

    ptr36_t expected_phys = 0x1000;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(simple_megapage){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, true);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(phys, mapped_phys);
}

PCUT_TEST(simple_megapage_non_base_page_mapping){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, true);

    ptr36_t mapped_phys = 0xFF;
    uint32_t requested_virt = 0x1000;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &mapped_phys);

    ptr36_t expected_phys = 0x1000;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(simple_global){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;
    unsigned different_asid = 2;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, true, false);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, different_asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(phys, mapped_phys);
}

PCUT_TEST(wrong_asid){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;
    unsigned different_asid = 2;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, different_asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success);
}

PCUT_TEST(unmapped_addr){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    uint32_t different_virt = 0xFFF0000;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, different_virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success);   
}

PCUT_TEST(megapage_priority){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    ptr36_t different_phys = 0xF0000000;
    unsigned asid = 1;

    // KTLB mapping
    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);
    // MTLB mapping
    rv_tlb_add_mapping(&tlb, asid, virt, different_phys, false, true);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(different_phys, mapped_phys);
}

PCUT_TEST(flush_all){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, false, false);

    rv_tlb_flush(&tlb);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success);
}

PCUT_TEST(flush_by_asid){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    ptr36_t phys = 0x0;
    unsigned asid1 = 1;
    unsigned asid2 = 2;

    rv_tlb_add_mapping(&tlb, asid1, virt1, phys, false, false);
    rv_tlb_add_mapping(&tlb, asid2, virt2, phys, false, false);

    rv_tlb_flush_by_asid(&tlb, asid1);

    ptr36_t mapped_phys = 0xFF;

    bool success1 = rv_tlb_get_mapping(&tlb, asid1, virt1, &mapped_phys);
    bool success2 = rv_tlb_get_mapping(&tlb, asid2, virt2, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
}

PCUT_TEST(flush_by_addr){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt1, phys, false, false);
    rv_tlb_add_mapping(&tlb, asid, virt2, phys, false, false);

    rv_tlb_flush_by_addr(&tlb, virt1);

    ptr36_t mapped_phys = 0xFF;

    bool success1 = rv_tlb_get_mapping(&tlb, asid, virt1, &mapped_phys);
    bool success2 = rv_tlb_get_mapping(&tlb, asid, virt2, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
}

PCUT_TEST(flush_by_asid_and_addr){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    uint32_t virt3 = 0x2000;
    ptr36_t phys = 0x0;
    unsigned asid1 = 1;
    unsigned asid2 = 2;

    rv_tlb_add_mapping(&tlb, asid1, virt1, phys, false, false);
    rv_tlb_add_mapping(&tlb, asid1, virt2, phys, false, false);
    rv_tlb_add_mapping(&tlb, asid2, virt3, phys, false, false);

    rv_tlb_flush_by_asid_and_addr(&tlb, asid1, virt1);

    ptr36_t mapped_phys = 0xFF;

    bool success1 = rv_tlb_get_mapping(&tlb, asid1, virt1, &mapped_phys);
    bool success2 = rv_tlb_get_mapping(&tlb, asid1, virt2, &mapped_phys);
    bool success3 = rv_tlb_get_mapping(&tlb, asid2, virt3, &mapped_phys);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
    PCUT_ASSERT_EQUALS(true, success3);
}

PCUT_TEST(flush_by_asid_ignores_global){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, true, false);

    rv_tlb_flush_by_asid(&tlb, asid);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(true, success);
}

PCUT_TEST(flush_by_asid_and_addr_ignores_global){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    rv_tlb_add_mapping(&tlb, asid, virt, phys, true, false);

    rv_tlb_flush_by_asid_and_addr(&tlb, asid, virt);

    ptr36_t mapped_phys = 0xFF;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &mapped_phys);

    PCUT_ASSERT_EQUALS(true, success);
}

PCUT_EXPORT(tlb);