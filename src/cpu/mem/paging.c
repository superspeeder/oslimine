//
// Created by andy on 1/3/26.
//

#include "paging.h"

#include "std/mem.h"

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
    PAGE_XD       = 1 << 63,
};

typedef struct __attribute__((packed)) page_table_t {
    uint64_t entries[512];
} page_table_t;

#define PTFROMENTRY(entry) ((page_table_t *)((entry) & (~0xfffULL)))

static page_table_t *get_base_page_table() {
    uint64_t value;
    asm volatile("mov %%cr3, %%rax" : "=a"(value));
    return PTFROMENTRY(value);
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

/// These are used to setup page tables which can map new page table entries.

static page_table_t __attribute__((aligned(4096))) interim_tables[4]; 

void init_paging() {
    memset(&(interim_tables[0]), 0, 4 * sizeof(page_table_t));
}


void *mmap_identity(const uintptr_t address, const size_t size, mmap_info_t *mmap_info) {
    return mmap(address, address, size, mmap_info);
}

void *mmap(const uintptr_t paddr, const uintptr_t vaddr, const size_t size, mmap_info_t *mmap_info) {
    uint64_t l4 = l4_index(vaddr);
    uint64_t l3 = l3_index(vaddr);
    uint64_t l2 = l2_index(vaddr);
    uint64_t l1 = l1_index(vaddr);

    page_table_t* pml4 = get_base_page_table();
    if ((pml4->entries[l4] & PAGE_PRESENT) == 0) {
        
    }

    return nullptr;
    // TODO
}
