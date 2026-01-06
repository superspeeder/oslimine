//
// Created by andy on 1/3/26.
//

#include "paging.h"

#include "boot.h"
#include "cpu/mem/phys.h"
#include "std/mem.h"
#include "trace.h"

enum {
    PAGE_PRESENT  = 0b1,
    PAGE_RW       = 0b10,
    PAGE_US       = 0b100,
    PAGE_PWT      = 0b1000,
    PAGE_PCD      = 0b10000,
    PAGE_ACCESSED = 0b100000,
    PAGE_DIRTY    = 0b1000000,
    PAGE_PS       = 0b10000000,
    PAGE_GLOBAL   = 0b100000000,
    PAGE_XD       = 1ULL << 63,
};

typedef struct __attribute__((packed)) page_table_t {
    uint64_t entries[512];
} page_table_t;

#define PTFROMENTRY(entry) ((page_table_t *)((entry) & (~0xfffULL)))

static page_table_t *get_base_page_table() {
    uint64_t value;
    asm volatile("mov %%cr3, %%rax" : "=a"(value));
    return (page_table_t*)((uintptr_t)PTFROMENTRY(value) + 0xffff'8000'0000'0000ULL);
}

/*
+----------------+------------+------------+------------+------------+---------------+
| 63-48 (unused) | 47-39 (l4) | 38-30 (l3) | 29-21 (l2) | 20-12 (l1) | 11-0 (unused) |
+----------------+------------+------------+------------+------------+---------------+
*/

static uint64_t l4_index(uintptr_t addr) {
    return (addr >> 39) & 0x1ff;
}

static uint64_t l3_index(uintptr_t addr) {
    return (addr >> 30) & 0x1ff;
}

static uint64_t l2_index(uintptr_t addr) {
    return (addr >> 21) & 0x1ff;
}

static uint64_t l1_index(uintptr_t addr) {
    return (addr >> 12) & 0x1ff;
}

static page_table_t interim_temp_pages[3];

void init_paging() {
    memset(&interim_temp_pages[0], 0, sizeof(interim_temp_pages));
}


uintptr_t translate_virt_to_phys(uintptr_t vaddr) {
    uint64_t l4 = l4_index(vaddr);
    uint64_t l3 = l3_index(vaddr);
    uint64_t l2 = l2_index(vaddr);
    uint64_t l1 = l1_index(vaddr);
    page_table_t *pml4 = get_base_page_table();

    if ((pml4->entries[l4] & PAGE_PRESENT) == 0) {
        return 0;
    }

    page_table_t *pdpt = PTFROMENTRY(pml4->entries[l4]);
    if ((pdpt->entries[l3] & PAGE_PRESENT) == 0) {
        return 0;
    }

    if ((pdpt->entries[l3] & PAGE_PS) != 0) {
        return (uintptr_t)PTFROMENTRY(pdpt->entries[l3]) + (vaddr & 0x3fffffff);        
    }

    page_table_t *pdt = PTFROMENTRY(pdpt->entries[l3]);
    if ((pdt->entries[l2] & PAGE_PRESENT) == 0) {
        return 0;
    }

    if ((pdt->entries[l2] & PAGE_PS) != 0) {
        return (uintptr_t)PTFROMENTRY(pdt->entries[l2]) + (vaddr & 0x1fffff);        
    }

    page_table_t *pt = PTFROMENTRY(pdt->entries[l2]);
    if ((pt->entries[l1] & PAGE_PRESENT) == 0) {
        return 0;
    }

    return (uintptr_t)PTFROMENTRY(pt->entries[l1]) + (vaddr & 0xfff);
}

void ptsetentry(page_table_t* pt, uint16_t idx, uintptr_t vaddr, uint64_t flags) {
    pt->entries[idx] = translate_virt_to_phys(vaddr) | flags;
}

// only maps 4k
static void tempmap(uintptr_t paddr, uintptr_t vaddr) {
    trace_enter();

    uint64_t l4 = l4_index(vaddr);
    uint64_t l3 = l3_index(vaddr);
    uint64_t l2 = l2_index(vaddr);
    uint64_t l1 = l1_index(vaddr);

    page_table_t *pml4 = get_base_page_table();
    if ((pml4->entries[l4] & PAGE_PRESENT) == 0) {
        trace("p4 not present");
        page_table_t *pdpt = &interim_temp_pages[0];
        pml4->entries[l4]  = (uint64_t)pdpt | PAGE_PRESENT | PAGE_RW;
        traceint("set pml4 entry", l4);
        traceinth("pdpt addr", (uint64_t)pdpt);
    } else {
        trace("p4 present");
    }

    page_table_t *pdpt = PTFROMENTRY(pml4->entries[l4]);
    if ((pdpt->entries[l3] & PAGE_PRESENT) == 0) {
        trace("p3 not present");
        page_table_t *pdt = &interim_temp_pages[1];
        pdpt->entries[l3] = (uint64_t)pdt | PAGE_PRESENT | PAGE_RW;
        traceint("set pdpt entry", l3);
        traceinth("pdt addr", (uint64_t)pdt);
    } else {
        trace("p3 present");
    }

    page_table_t *pdt = PTFROMENTRY(pdpt->entries[l3]);
    if ((pdt->entries[l3] & PAGE_PRESENT) == 0) {
        trace("p2 not present");
        page_table_t *pt = &interim_temp_pages[2];
        pdt->entries[l2] = (uint64_t)pt | PAGE_PRESENT | PAGE_RW;
        traceint("set pdt entry", l2);
    } else {
        trace("p2 present");
    }

    page_table_t *pt = PTFROMENTRY(pdt->entries[l2]);
    traceinth("pt", (uintptr_t)pt);

    traceint("set pt entry", l1);
    pt->entries[l1]  = paddr | PAGE_PRESENT | PAGE_RW;
    traceinth("paddr", paddr);

    trace_exit();
}

static void tempunmap(uintptr_t paddr, uintptr_t vaddr) {
    trace_enter();
    uint64_t l4 = l4_index(vaddr);
    uint64_t l3 = l3_index(vaddr);
    uint64_t l2 = l2_index(vaddr);
    uint64_t l1 = l1_index(vaddr);

    page_table_t *pml4 = get_base_page_table();
    page_table_t *pdpt = PTFROMENTRY(pml4->entries[l4]);
    if (!pdpt) {
        trace_exit();
        return;
    }

    page_table_t *pdt = PTFROMENTRY(pdpt->entries[l3]);
    if (!pdt) {
        trace_exit();
        return;
    }

    page_table_t *pt = PTFROMENTRY(pdpt->entries[l2]);
    if (!pt) {
        trace_exit();
        return;
    }

    pt->entries[l1] = 0;

    if (pt == &interim_temp_pages[2]) {
        pdt->entries[l2] = 0;
    }

    if (pdt == &interim_temp_pages[1]) {
        pdpt->entries[l3] = 0;
    }

    if (pdpt == &interim_temp_pages[0]) {
        pml4->entries[l4] = 0;
    }

    trace_exit();
}

void *mmap_identity(const uintptr_t address, const size_t size, mmap_info_t *mmap_info) {
    return mmap(address, address, size, mmap_info);
}

/**
 * @brief Map memory
 *
 * paddr and vaddr must be 4K aligned, or else this function will not work properly
 *
 * @param paddr
 * @param vaddr
 * @param size
 * @param mmap_info
 * @return void*
 */

// TODO: Prevent stepping on existing mappings.
void *mmap(uintptr_t paddr, uintptr_t vaddr, const size_t size, mmap_info_t *mmap_info) {
    trace_enter();
    if (paddr == 0L || vaddr == 0L) {
        trace_exit();
        return nullptr;
    }

    uintptr_t vend    = vaddr + size;
    uintptr_t vcursor = vaddr;

    page_table_t *pml4 = get_base_page_table();
    traceinth("pml4a", (uint64_t)pml4);

    while (vcursor < vend) {
        uint64_t l4 = l4_index(vaddr);
        uint64_t l3 = l3_index(vaddr);
        uint64_t l2 = l2_index(vaddr);
        uint64_t l1 = l1_index(vaddr);

        if ((pml4->entries[l4] & PAGE_PRESENT) == 0) {
            page_table_t *pdpt = (page_table_t*)palloc(sizeof(page_table_t));
            tempmap((uintptr_t)pdpt, (uintptr_t)pdpt);
            pml4->entries[l4] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_RW;
        }

        page_table_t *pdpt = PTFROMENTRY(pml4->entries[l4]);

        // virtual and physical addr must be properly aligned for this to work
        if ((vaddr & 0x3fffffff) == 0 && (paddr & 0x3fffffff) == 0 && size >= 0x40000000) {
            pdpt->entries[l3] = paddr | PAGE_PRESENT | PAGE_RW | PAGE_PS;
            vcursor += 0x40000000;
            trace("map entry 1g");
            continue;
        } else if ((pdpt->entries[l3] & PAGE_PRESENT) == 0) {
            page_table_t *pdt = (page_table_t*)palloc(sizeof(page_table_t));
            tempmap((uintptr_t)pdt, (uintptr_t)pdt);
            pdpt->entries[l3] = (uint64_t)pdt | PAGE_PRESENT | PAGE_RW;
        }

        page_table_t *pdt = PTFROMENTRY(pdpt->entries[l3]);

        if ((vaddr & 0x1fffff) == 0 && (paddr & 0x1fffff) == 0 && size >= 0x200000) {
            pdt->entries[l2] = paddr | PAGE_PRESENT | PAGE_RW | PAGE_PS;
            vcursor += 0x200000;
            trace("map entry 2m");
            continue;
        } else if ((pdt->entries[l2] & PAGE_PRESENT) == 0) {
            page_table_t *pt = (page_table_t*)palloc(sizeof(page_table_t));
            tempmap((uintptr_t)pt, (uintptr_t)pt);
            pdt->entries[l2] = (uint64_t)pt | PAGE_PRESENT | PAGE_RW;
        }

        page_table_t *pt = PTFROMENTRY(pdt->entries[l2]);
        pt->entries[l1]  = paddr | PAGE_PRESENT | PAGE_RW;
        vcursor += 0x1000;
        trace("map entry 4k");
    }
    trace_exit();
    return (void *)vaddr;
}