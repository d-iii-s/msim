#include <string.h>

#include "../../../assert.h"
#include "../../../utils.h"
#include "tlb.h"
#include "virt_mem.h"

typedef struct rv64_tlb_entry {
    item_t item; // Item to be used in LRU or free-list
    sv39_pte_t pte;
    uint64_t vpn;
    unsigned asid;
    bool global;
    sv39_page_type_t page_type;
} rv64_tlb_entry_t;

/** Caches a mapping into the TLB */
extern void rv64_tlb_add_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt, sv39_pte_t pte, sv39_page_type_t page_type, bool global)
{
    rv64_tlb_entry_t *entry = NULL;

    if (!is_empty(&tlb->free_list)) {
        // If there are some unused entries, use them first

        item_t *popped_free_item = tlb->free_list.head;
        list_remove(&tlb->free_list, popped_free_item);
        // Safe cast because the item is the first field
        entry = (rv64_tlb_entry_t *) popped_free_item;
    } else {
        // If all entries are used, reuse the Least Recently Used entry from the used list
        item_t *popped_reused_item = tlb->lru_list.tail;
        list_remove(&tlb->lru_list, popped_reused_item);
        // Safe cast because the item is the first field
        entry = (rv64_tlb_entry_t *) popped_reused_item;
    }

    ASSERT(entry != NULL);

    uint64_t vpn;

    switch (page_type) {
    case page:
        vpn = virt >> RV64_PAGESIZE;
        break;
    case megapage:
        vpn = virt >> RV64_MEGAPAGESIZE;
        break;
    case gigapage:
        vpn = virt >> RV64_GIGAPAGESIZE;
        break;
    default:
        return;
    }

    entry->pte = pte;
    entry->page_type = page_type;
    entry->vpn = vpn;
    entry->asid = asid;
    entry->global = global;

    // Push to front of LRU list
    list_push(&tlb->lru_list, &entry->item);
}

static void move_lru_entry_to_front(rv64_tlb_t *tlb, rv64_tlb_entry_t *entry)
{
    // Fast path exit
    if (tlb->lru_list.head == &entry->item) {
        return;
    }

    ASSERT(entry->item.list == &tlb->lru_list);

    // Move entry to the front of the list by first removing it and then pushing it to the front
    list_remove(&tlb->lru_list, &entry->item);
    list_push(&tlb->lru_list, &entry->item);
}

/** Returns whether the given TLB entry holds a mapping of the specified virtual address */
static bool entry_maps_virt(rv64_tlb_entry_t *entry, uint64_t virt)
{
    uint64_t vpn;

    switch (entry->page_type) {
    case page:
        vpn = virt >> RV64_PAGESIZE;
        break;
    case megapage:
        vpn = virt >> RV64_MEGAPAGESIZE;
        break;
    case gigapage:
        vpn = virt >> RV64_GIGAPAGESIZE;
        break;
    default:
        return false;
    }

    return entry->vpn == vpn;
}

/** Retrieves a cached mapping
 * gives priority to megapage mappings
 */
extern bool rv64_tlb_get_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt, sv39_pte_t *pte, sv39_page_type_t *page_type, bool noisy)
{
    rv64_tlb_entry_t *entry;

    for_each(tlb->lru_list, entry, rv64_tlb_entry_t)
    {
        // Skip non-global mappings with wrong asid
        if (!entry->global && entry->asid != asid) {
            continue;
        }

        if (entry_maps_virt(entry, virt)) {
            if (noisy) {
                // Ensure LRU behavior, by moving the entry to the front of the LRU list on access
                move_lru_entry_to_front(tlb, entry);
            }

            *pte = entry->pte;
            *page_type = entry->page_type;

            return true;
        }
    }

    return false;
}

static void invalidate_tlb_entry(rv64_tlb_t *tlb, rv64_tlb_entry_t *entry)
{
    ASSERT(entry->item.list == &tlb->lru_list);

    list_remove(&tlb->lru_list, &entry->item);
    list_push(&tlb->free_list, &entry->item);
}

static bool is_entry_valid(rv64_tlb_t *tlb, rv64_tlb_entry_t *entry)
{
    return entry->item.list == &tlb->lru_list;
}

extern void rv64_tlb_remove_mapping(rv64_tlb_t *tlb, unsigned asid, uint64_t virt)
{
    rv64_tlb_entry_t *entry;

    for_each(tlb->lru_list, entry, rv64_tlb_entry_t)
    {
        // Skip non-global mappings with wrong asid
        if (!entry->global && entry->asid != asid) {
            continue;
        }

        if (entry_maps_virt(entry, virt)) {
            invalidate_tlb_entry(tlb, entry);
            return;
        }
    }
}

/** TLB flushes */

// Invalidates all entries
extern void rv64_tlb_flush(rv64_tlb_t *tlb)
{
    for (size_t i = 0; i < tlb->size; ++i) {
        if (is_entry_valid(tlb, &tlb->entries[i])) {
            invalidate_tlb_entry(tlb, &tlb->entries[i]);
        }
    }
}

// Invalidates all entries of the given asid
extern void rv64_tlb_flush_by_asid(rv64_tlb_t *tlb, unsigned asid)
{
    for (size_t i = 0; i < tlb->size; ++i) {

        if (!is_entry_valid(tlb, &tlb->entries[i])) {
            continue;
        }

        if (tlb->entries[i].global) {
            continue;
        }
        if (tlb->entries[i].asid == asid) {
            invalidate_tlb_entry(tlb, &tlb->entries[i]);
        }
    }
}

// Invalidates all entries that map the given virtual address
extern void rv64_tlb_flush_by_addr(rv64_tlb_t *tlb, uint64_t virt)
{
    uint64_t page_vpn = virt >> RV64_PAGESIZE;
    uint64_t mega_vpn = virt >> RV64_MEGAPAGESIZE;
    uint64_t giga_vpn = virt >> RV64_GIGAPAGESIZE;

    for (size_t i = 0; i < tlb->size; ++i) {
        if (!is_entry_valid(tlb, &tlb->entries[i])) {
            continue;
        }

        switch (tlb->entries[i].page_type) {
        case page:
            if (tlb->entries[i].vpn == page_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        case megapage:
            if (tlb->entries[i].vpn == mega_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        case gigapage:
            if (tlb->entries[i].vpn == giga_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        }
    }
}

// Invalidates all entries that map the given address and are of the given asid
extern void rv64_tlb_flush_by_asid_and_addr(rv64_tlb_t *tlb, unsigned asid, uint64_t virt)
{
    uint64_t page_vpn = virt >> RV64_PAGESIZE;
    uint64_t mega_vpn = virt >> RV64_MEGAPAGESIZE;
    uint64_t giga_vpn = virt >> RV64_GIGAPAGESIZE;

    for (size_t i = 0; i < tlb->size; ++i) {
        if (!is_entry_valid(tlb, &tlb->entries[i])) {
            continue;
        }

        if (tlb->entries[i].global) {
            continue;
        }

        if (tlb->entries[i].asid != asid) {
            continue;
        }

        switch (tlb->entries[i].page_type) {
        case page:
            if (tlb->entries[i].vpn == page_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        case megapage:
            if (tlb->entries[i].vpn == mega_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        case gigapage:
            if (tlb->entries[i].vpn == giga_vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
            break;
        }
    }
}

/** Initializes the TLB data structure */
extern void rv64_tlb_init(rv64_tlb_t *tlb, size_t size)
{
    ASSERT(size != 0);

    tlb->entries = safe_malloc(size * sizeof(rv64_tlb_entry_t));
    tlb->size = size;
    list_init(&tlb->lru_list);
    list_init(&tlb->free_list);

    memset(tlb->entries, 0, size * sizeof(rv64_tlb_entry_t));

    for (size_t i = 0; i < size; ++i) {
        list_append(&tlb->free_list, &tlb->entries[i].item);
    }
}

/** Cleans up the TLB structure */
extern void rv64_tlb_done(rv64_tlb_t *tlb)
{
    safe_free(tlb->entries);
}

extern bool rv64_tlb_resize(rv64_tlb_t *tlb, size_t size)
{
    safe_free(tlb->entries);
    tlb->entries = safe_malloc(size * sizeof(rv64_tlb_entry_t));
    tlb->size = size;
    list_init(&tlb->lru_list);
    list_init(&tlb->free_list);

    memset(tlb->entries, 0, size * sizeof(rv64_tlb_entry_t));

    for (size_t i = 0; i < size; ++i) {
        list_append(&tlb->free_list, &tlb->entries[i].item);
    }

    return true;
}

static inline void dump_tlb_entry(rv64_tlb_entry_t entry, string_t *text)
{
    uint64_t virt;

    switch (entry.page_type) {
    case page:
        virt = entry.vpn << RV64_PAGESIZE;
        break;
    case megapage:
        virt = entry.vpn << RV64_MEGAPAGESIZE;
        break;
    case gigapage:
        virt = entry.vpn << RV64_GIGAPAGESIZE;
        break;
    default:
        return;
    }

    string_printf(text, "0x%08lx => 0x%09lx [ ASID: %d, GLOBAL: %s, PAGE TYPE: %s ]",
            virt,
            (ptr55_t) entry.pte.ppn << RV64_PAGESIZE,
            entry.asid,
            entry.global ? "T" : "F",
            entry.page_type == page ? "P" : (entry.page_type == megapage ? "M" : "G"));
}

extern void rv64_tlb_dump(rv64_tlb_t *tlb)
{
    string_t s_text;
    string_init(&s_text);

    printf("TLB    size: %ld entries\n", tlb->size);
    printf("%8s: %10s => %-11s [ %s ]\n", "index", "virt", "phys", "info");

    bool printed = false;

    rv64_tlb_entry_t *entry;
    size_t i = 0;

    for_each(tlb->lru_list, entry, rv64_tlb_entry_t)
    {
        printed = true;

        string_clear(&s_text);
        dump_tlb_entry(*entry, &s_text);
        printf("%8ld: %s\n", i, s_text.str);

        ++i;
    }

    if (!printed) {
        printf("\t Empty\n");
    }

    string_done(&s_text);
}
