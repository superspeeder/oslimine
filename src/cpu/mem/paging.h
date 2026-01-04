//
// Created by andy on 1/3/26.
//

#pragma once
#include <stdint.h>
#include <stddef.h>

void init_paging();

typedef struct mmap_info_t {
    void *base;
    size_t size;
    uintptr_t paddr;
} mmap_info_t;

void *mmap_identity(uintptr_t address, size_t size, mmap_info_t *mmap_info);
void *mmap(uintptr_t paddr, uintptr_t vaddr, size_t size, mmap_info_t *mmap_info);
