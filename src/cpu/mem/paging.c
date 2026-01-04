//
// Created by andy on 1/3/26.
//

#include "paging.h"

void init_paging() {
}

void * mmap_identity(const uintptr_t address, const size_t size, mmap_info_t *mmap_info) {
    return mmap(address, address, size, mmap_info);
}

void * mmap(const uintptr_t paddr, const uintptr_t vaddr, const size_t size, mmap_info_t *mmap_info) {

    return nullptr;
    // TODO
}
