#include <string.h>

#include "../../../assert.h"
#include "../../../utils.h"
#include "tlb.h"

typedef struct rv_tlb_entry {
    item_t item; // Item to be used in LRU or free-list
    sv32_pte_t pte;
    uint32_t vpn;
    unsigned asid;
    bool global;
    bool megapage;
} rv_tlb_entry_t;

/** Caches a mapping into the TLB */
extern void rv_tlb_add_mapping(rv_tlb_t *tlb, unsigned asid, uint32_t virt, sv32_pte_t pte, bool megapage, bool global)
{

    rv_tlb_entry_t *entry = NULL;

    if (!is_empty(&tlb->free_list)) {
        // If there are some unused entries, use them first

        item_t *popped_free_item = tlb->free_list.head;
        list_remove(&tlb->free_list, popped_free_item);
        // Safe cast because the item is the first field
        entry = (rv_tlb_entry_t *) popped_free_item;
    } else {
        // If all entries are used, reuse the Least Recently Used entry from the used list

        item_t *popped_reused_item = tlb->lru_list.tail;
        list_remove(&tlb->lru_list, popped_reused_item);
        // Safe cast because the item is the first field
        entry = (rv_tlb_entry_t *) popped_reused_item;
    }

    ASSERT(entry != NULL);

    entry->pte = pte;
    entry->megapage = megapage;
    entry->vpn = virt >> (megapage ? RV_MEGAPAGESIZE : RV_PAGESIZE);
    entry->asid = asid;
    entry->global = global;

    // Push to front of LRU list
    list_push(&tlb->lru_list, &entry->item);
}

static void move_lru_entry_to_front(rv_tlb_t *tlb, rv_tlb_entry_t *entry)
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
static bool entry_maps_virt(rv_tlb_entry_t *entry, uint32_t virt)
{
    uint32_t vpn = virt >> RV_PAGESIZE;
    uint32_t mvpn = virt >> RV_MEGAPAGESIZE;
    return entry->vpn == (entry->megapage ? mvpn : vpn);
}

/** Retrieves a cached mapping
 * gives priority to megapage mappings
 */
extern bool rv_tlb_get_mapping(rv_tlb_t *tlb, unsigned asid, uint32_t virt, sv32_pte_t *pte, bool *megapage, bool noisy)
{

    rv_tlb_entry_t *entry;

    for_each(tlb->lru_list, entry, rv_tlb_entry_t)
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
            *megapage = entry->megapage;

            return true;
        }
    }

    return false;
}

static void invalidate_tlb_entry(rv_tlb_t *tlb, rv_tlb_entry_t *entry)
{
    ASSERT(entry->item.list == &tlb->lru_list);

    list_remove(&tlb->lru_list, &entry->item);
    list_push(&tlb->free_list, &entry->item);
}

static bool is_entry_valid(rv_tlb_t *tlb, rv_tlb_entry_t *entry)
{
    return entry->item.list == &tlb->lru_list;
}

extern void rv_tlb_remove_mapping(rv_tlb_t *tlb, unsigned asid, uint32_t virt)
{

    rv_tlb_entry_t *entry;

    for_each(tlb->lru_list, entry, rv_tlb_entry_t)
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
extern void rv_tlb_flush(rv_tlb_t *tlb)
{
    for (size_t i = 0; i < tlb->size; ++i) {
        if (is_entry_valid(tlb, &tlb->entries[i])) {
            invalidate_tlb_entry(tlb, &tlb->entries[i]);
        }
    }
}

// Invalidates all entries of the given asid
extern void rv_tlb_flush_by_asid(rv_tlb_t *tlb, unsigned asid)
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
extern void rv_tlb_flush_by_addr(rv_tlb_t *tlb, uint32_t virt)
{
    uint32_t vpn = virt >> RV_PAGESIZE;
    uint32_t mvpn = virt >> RV_MEGAPAGESIZE;

    for (size_t i = 0; i < tlb->size; ++i) {

        if (!is_entry_valid(tlb, &tlb->entries[i])) {
            continue;
        }

        if (tlb->entries[i].megapage) {
            if (tlb->entries[i].vpn == mvpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
        } else {
            if (tlb->entries[i].vpn == vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
        }
    }
}

// Invalidates all entries that map the given address and are of the given asid
extern void rv_tlb_flush_by_asid_and_addr(rv_tlb_t *tlb, unsigned asid, uint32_t virt)
{
    uint32_t vpn = virt >> RV_PAGESIZE;
    uint32_t mvpn = virt >> RV_MEGAPAGESIZE;

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

        if (tlb->entries[i].megapage) {
            if (tlb->entries[i].vpn == mvpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
        } else {
            if (tlb->entries[i].vpn == vpn) {
                invalidate_tlb_entry(tlb, &tlb->entries[i]);
            }
        }
    }
}

/** Initializes the TLB data structure */
extern void rv_tlb_init(rv_tlb_t *tlb, size_t size)
{
    ASSERT(size != 0);

    tlb->entries = safe_malloc(size * sizeof(rv_tlb_entry_t));
    tlb->size = size;
    list_init(&tlb->lru_list);
    list_init(&tlb->free_list);

    memset(tlb->entries, 0, size * sizeof(rv_tlb_entry_t));

    for (size_t i = 0; i < size; ++i) {
        list_append(&tlb->free_list, &tlb->entries[i].item);
    }
}

/** Cleans up the TLB structure */
extern void rv_tlb_done(rv_tlb_t *tlb)
{
    safe_free(tlb->entries);
}

extern bool rv_tlb_resize(rv_tlb_t *tlb, size_t size)
{
    safe_free(tlb->entries);
    tlb->entries = safe_malloc(size * sizeof(rv_tlb_entry_t));
    tlb->size = size;
    list_init(&tlb->lru_list);
    list_init(&tlb->free_list);

    memset(tlb->entries, 0, size * sizeof(rv_tlb_entry_t));

    for (size_t i = 0; i < size; ++i) {
        list_append(&tlb->free_list, &tlb->entries[i].item);
    }

    return true;
}

static inline void dump_tlb_entry(rv_tlb_entry_t entry, string_t *text)
{
    string_printf(text, "0x%08x => 0x%09lx [ ASID: %d, GLOBAL: %s, MEGAPAGE: %s ]",
            entry.vpn << (entry.megapage ? RV_MEGAPAGESIZE : RV_PAGESIZE),
            (ptr36_t) entry.pte.ppn << RV_PAGESIZE,
            entry.asid,
            entry.global ? "T" : "F",
            entry.megapage ? "T" : "F");
}

extern void rv_tlb_dump(rv_tlb_t *tlb)
{
    string_t s_text;
    string_init(&s_text);

    printf("TLB    size: %ld entries\n", tlb->size);
    printf("%8s: %10s => %-11s [ %s ]\n", "index", "virt", "phys", "info");

    bool printed = false;

    rv_tlb_entry_t *entry;
    size_t i = 0;

    for_each(tlb->lru_list, entry, rv_tlb_entry_t)
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
