#include <string.h>

#include "tlb.h"
#include "../../../utils.h"

typedef struct rv_tlb_entry {
    sv32_pte_t pte;
    uint32_t vpn;
    unsigned asid;
    bool valid;
    bool global;
} rv_tlb_entry_t;

static inline size_t hash(size_t num){
    // No hashing as of now
    return num;
}

#define index_ktlb(tlb, index) \
    ((tlb)->ktlb[hash((index)) % ((tlb)->ktlb_size)])

#define index_mtlb(tlb, index) \
    ((tlb)->mtlb[hash((index)) % ((tlb)->mtlb_size)])

/** Caches a mapping into the TLB */
extern void rv_tlb_add_mapping(rv_tlb_t* tlb, unsigned asid, uint32_t virt, sv32_pte_t pte, bool megapage, bool global){
    
    uint32_t vpn = virt >> RV_PAGESIZE;
    
    if(megapage){
        uint32_t mvpn = virt >> RV_MEGAPAGESIZE;

        index_mtlb(tlb, mvpn).vpn = mvpn;
        index_mtlb(tlb, mvpn).pte = pte;
        index_mtlb(tlb, mvpn).global = global;
        index_mtlb(tlb, mvpn).asid = asid;
        index_mtlb(tlb, mvpn).valid = true;    
    }
    else {
        index_ktlb(tlb, vpn).vpn = vpn;
        index_ktlb(tlb, vpn).pte = pte;
        index_ktlb(tlb, vpn).global = global;
        index_ktlb(tlb, vpn).asid = asid;
        index_ktlb(tlb, vpn).valid = true;
    }
}

/** Retrieves a cached mapping
 * gives priority to megapage mappings
 */
extern bool rv_tlb_get_mapping(rv_tlb_t* tlb, unsigned asid, uint32_t virt, sv32_pte_t* pte, bool* megapage){
    uint32_t vpn = virt >> 12;
    uint32_t mvpn = vpn >> 10;

    // Megapages have priority
    if(index_mtlb(tlb, mvpn).valid && index_mtlb(tlb, mvpn).vpn == mvpn){
        if(index_mtlb(tlb, mvpn).global || index_mtlb(tlb, mvpn).asid == asid){
            *pte = index_mtlb(tlb, mvpn).pte;
            *megapage = true;
            return true;
        }
    }

    if(index_ktlb(tlb, vpn).valid && index_ktlb(tlb, vpn).vpn == vpn){
        if(index_ktlb(tlb, vpn).global || index_ktlb(tlb, vpn).asid == asid){
            *pte = index_ktlb(tlb, vpn).pte;
            *megapage = false;
            return true;
        }
    }
    
    return false;
}

/** TLB flushes */

// Invalidates all entries
extern void rv_tlb_flush(rv_tlb_t* tlb){
    for(size_t i = 0; i < tlb->ktlb_size; ++i){
        tlb->ktlb[i].valid = false;
    }
    for(size_t i = 0; i < tlb->mtlb_size; ++i){
        tlb->mtlb[i].valid = false;
    }
}

// Invalidates all entries of the given asid
extern void rv_tlb_flush_by_asid(rv_tlb_t* tlb, unsigned asid){
    for(size_t i = 0; i < tlb->ktlb_size; ++i){
        if(tlb->ktlb[i].global)
            continue;
        if(tlb->ktlb[i].asid == asid)
            tlb->ktlb[i].valid = false;
    }
    for(size_t i = 0; i < tlb->mtlb_size; ++i){
        if(tlb->mtlb[i].global)
            continue;
        if(tlb->mtlb[i].asid == asid)
            tlb->mtlb[i].valid = false;
    }
}

// Invalidates all entries that map the given virtual address
extern void rv_tlb_flush_by_addr(rv_tlb_t* tlb, uint32_t virt){
    uint32_t vpn = virt >> RV_PAGESIZE;
    uint32_t mvpn = virt >> RV_MEGAPAGESIZE;
    
    index_ktlb(tlb, vpn).valid = false;
    index_mtlb(tlb, mvpn).valid = false;
}

// Invalidates all entries that map the given address and are of the given asid
extern void rv_tlb_flush_by_asid_and_addr(rv_tlb_t* tlb, unsigned asid, uint32_t virt){
    uint32_t vpn = virt >> RV_PAGESIZE;
    uint32_t mvpn = virt >> RV_MEGAPAGESIZE;
    
    // Flush from KTLB if the mapping is not global and the asid matches
    if(!index_ktlb(tlb, vpn).global && index_ktlb(tlb, vpn).asid == asid)
        index_ktlb(tlb, vpn).valid = false;
    
    // Flush from MTLB if the mapping is not global and the asid matches
    if(!index_mtlb(tlb, mvpn).global && index_mtlb(tlb, mvpn).asid == asid)
        index_mtlb(tlb, mvpn).valid = false;
}

/** Initializes the TLB data structure */
extern bool rv_tlb_init(rv_tlb_t* tlb, size_t ktlb_size, size_t mtlb_size){
    if(ktlb_size == 0 || !IS_POWER_OF_2(ktlb_size) || mtlb_size == 0 || !IS_POWER_OF_2(mtlb_size)){
        return false;
    }
    tlb->ktlb = safe_malloc(ktlb_size * sizeof(rv_tlb_entry_t));
    tlb->ktlb_size = ktlb_size;
    tlb->mtlb = safe_malloc(mtlb_size * sizeof(rv_tlb_entry_t));
    tlb->mtlb_size = mtlb_size;

    memset(tlb->ktlb, 0, ktlb_size * sizeof(rv_tlb_entry_t));
    memset(tlb->mtlb, 0, mtlb_size * sizeof(rv_tlb_entry_t));

    return true;    
}

/** Cleans up the TLB structure */
extern void rv_tlb_done(rv_tlb_t* tlb){
    safe_free(tlb->ktlb);
    safe_free(tlb->mtlb);
}

extern bool rv_tlb_resize_ktlb(rv_tlb_t* tlb, size_t size){
    safe_free(tlb->ktlb);
    tlb->ktlb = safe_malloc(size * sizeof(rv_tlb_entry_t));
    tlb->ktlb_size = size;

    memset(tlb->ktlb, 0, size * sizeof(rv_tlb_entry_t));

    return true;
}
extern bool rv_tlb_resize_mtlb(rv_tlb_t* tlb, size_t size){
    safe_free(tlb->mtlb);
    tlb->mtlb = safe_malloc(size * sizeof(rv_tlb_entry_t));
    tlb->mtlb_size = size;

    memset(tlb->mtlb, 0, size * sizeof(rv_tlb_entry_t));

    return true;
}

static inline void dump_tlb_entry(rv_tlb_entry_t entry, string_t* text, bool megapage){
    if(!entry.valid){
        string_printf(text, "INVALID");
    }
    else{
        string_printf(text, "0x%08x => 0x%09lx [ ASID: %d, GLOBAL: %s ]",
            entry.vpn << (megapage ? 22 : 12),
            (ptr36_t)entry.pte.ppn << 12,
            entry.asid,
            entry.global ? "T" : "F"
        );
    }
}

extern void rv_tlb_dump(rv_tlb_t* tlb){
    string_t s_text;
    string_init(&s_text);

    printf("Kilo TLB\tsize: %ld entries\n", tlb->ktlb_size);
    printf("%8s: %10s => %-11s [ %s ]\n", "index", "virt", "phys", "info");

    bool printed = false;
    for(size_t i = 0; i < tlb->ktlb_size; ++i){
        // Print only valid entries to not overwhelm the debug output
        if(!tlb->ktlb[i].valid)
            continue;

        printed = true;
        
        string_clear(&s_text);
        dump_tlb_entry(tlb->ktlb[i], &s_text, false);
        printf("%8ld: %s\n", i, s_text.str);
    }

    if(!printed){
        printf("\t Empty\n");
    }

    printf("\nMega TLB\tsize: %ld entries\n", tlb->mtlb_size);
    printf("%8s: %10s => %-11s [ %s ]\n", "index", "virt", "phys", "info");
    printed = false;
    for(size_t i = 0; i < tlb->mtlb_size; ++i){
        // Print only valid entries to not overwhelm the debug output
        if(!tlb->mtlb[i].valid)
            continue;
        
        printed = true;
        string_clear(&s_text);
        dump_tlb_entry(tlb->mtlb[i], &s_text, true);
        printf("%8ld: %s\n", i, s_text.str);
    }

    if(!printed){
        printf("\t Empty\n");
    }

    string_done(&s_text);
}