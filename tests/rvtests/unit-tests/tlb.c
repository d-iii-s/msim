#include <stdint.h>
#include <stdio.h>
#include <pcut/pcut.h>
#include "../../../src/device/cpu/riscv_rv32ima/tlb.h"
#include "../../../src/device/cpu/riscv_rv32ima/virt_mem.h"

PCUT_INIT

PCUT_TEST_SUITE(tlb);

rv_tlb_t tlb;

PCUT_TEST_BEFORE {
    rv_tlb_init(&tlb, DEFAULT_RV_TLB_SIZE);
}

PCUT_TEST_AFTER {
    rv_tlb_done(&tlb);
}

PCUT_TEST(simple){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? (ptr36_t)pte.ppn << 12 : 0xFF;

    ptr36_t expected_phys = 0x0;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_EQUALS(false, megapage);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(get_with_offset){
    
    uint32_t virt = 0x0;
    ptr36_t phys  = 0x1000;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    uint32_t requested_virt = 0x0001;

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? (ptr36_t)pte.ppn << 12 : 0xFF;

    ptr36_t expected_phys = 0x1000;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(add_with_offset){
    
    uint32_t virt = 0x0001;
    ptr36_t phys  = 0x1000;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    uint32_t requested_virt = 0x0;

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? (ptr36_t)pte.ppn << 12 : 0xFF;

    ptr36_t expected_phys = 0x1000;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(simple_megapage){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, true, false);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? (ptr36_t)pte.ppn << 12 : 0xFF;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_EQUALS(true, megapage);
    PCUT_ASSERT_INT_EQUALS(phys, mapped_phys);
}

PCUT_TEST(simple_megapage_non_base_page_mapping){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, true, false);

    uint32_t requested_virt = 0x1000;

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, requested_virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? ((ptr36_t)pte.ppn) << 12 : 0xFF;

    ptr36_t expected_phys = 0x0000;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_EQUALS(true, megapage);
    PCUT_ASSERT_INT_EQUALS(expected_phys, mapped_phys);
}

PCUT_TEST(simple_global){
    
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;
    unsigned different_asid = 2;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = true;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, true);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, different_asid, virt, &pte, &megapage);

    ptr36_t mapped_phys = success ? (ptr36_t)pte.ppn << 12 : 0xFF;

    PCUT_ASSERT_EQUALS(true, success);
    PCUT_ASSERT_INT_EQUALS(phys, mapped_phys);
}

PCUT_TEST(wrong_asid){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;
    unsigned different_asid = 2;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = false;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, different_asid, virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success);
}

PCUT_TEST(unmapped_addr){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    uint32_t different_virt = 0xFFF0000;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, different_virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success);   
}

PCUT_TEST(flush_all){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, false);

    rv_tlb_flush(&tlb);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success);
}

PCUT_TEST(flush_all_global){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, true);

    rv_tlb_flush(&tlb);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success);
}

PCUT_TEST(flush_by_asid){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    ptr36_t phys = 0x0;
    unsigned asid1 = 1;
    unsigned asid2 = 2;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = false;

    rv_tlb_add_mapping(&tlb, asid1, virt1, added_pte, false, false);
    rv_tlb_add_mapping(&tlb, asid2, virt2, added_pte, false, false);

    rv_tlb_flush_by_asid(&tlb, asid1);

    sv32_pte_t pte;
    bool megapage;

    bool success1 = rv_tlb_get_mapping(&tlb, asid1, virt1, &pte, &megapage);
    bool success2 = rv_tlb_get_mapping(&tlb, asid2, virt2, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
}

PCUT_TEST(flush_by_addr){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;

    rv_tlb_add_mapping(&tlb, asid, virt1, added_pte, false, false);
    rv_tlb_add_mapping(&tlb, asid, virt2, added_pte, false, false);

    rv_tlb_flush_by_addr(&tlb, virt1);

    sv32_pte_t pte;
    bool megapage;

    bool success1 = rv_tlb_get_mapping(&tlb, asid, virt1, &pte, &megapage);
    bool success2 = rv_tlb_get_mapping(&tlb, asid, virt2, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
}

PCUT_TEST(flush_by_asid_and_addr){
    uint32_t virt1 = 0x0;
    uint32_t virt2 = 0x1000;
    uint32_t virt3 = 0x2000;
    ptr36_t phys = 0x0;
    unsigned asid1 = 42;
    unsigned asid2 = 43;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = false;

    rv_tlb_add_mapping(&tlb, asid1, virt1, added_pte, false, false);
    rv_tlb_add_mapping(&tlb, asid1, virt2, added_pte, false, false);
    rv_tlb_add_mapping(&tlb, asid2, virt3, added_pte, false, false);

    rv_tlb_flush_by_asid_and_addr(&tlb, asid1, virt1);

    sv32_pte_t pte;
    bool megapage;

    bool success1 = rv_tlb_get_mapping(&tlb, asid1, virt1, &pte, &megapage);
    bool success2 = rv_tlb_get_mapping(&tlb, asid1, virt2, &pte, &megapage);
    bool success3 = rv_tlb_get_mapping(&tlb, asid2, virt3, &pte, &megapage);

    PCUT_ASSERT_EQUALS(false, success1);
    PCUT_ASSERT_EQUALS(true, success2);
    PCUT_ASSERT_EQUALS(true, success3);
}

PCUT_TEST(flush_by_asid_ignores_global){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = true;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, true);

    rv_tlb_flush_by_asid(&tlb, asid);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(true, success);
}

PCUT_TEST(flush_by_asid_and_addr_ignores_global){
    uint32_t virt = 0x0;
    ptr36_t phys = 0x0;
    unsigned asid = 1;

    sv32_pte_t added_pte = { 0 };
    added_pte.ppn = phys >> 12;
    added_pte.g = true;

    rv_tlb_add_mapping(&tlb, asid, virt, added_pte, false, true);

    rv_tlb_flush_by_asid_and_addr(&tlb, asid, virt);

    sv32_pte_t pte;
    bool megapage;

    bool success = rv_tlb_get_mapping(&tlb, asid, virt, &pte, &megapage);

    PCUT_ASSERT_EQUALS(true, success);
}

PCUT_EXPORT(tlb);