//
// Created by andy on 1/3/26.
//

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/// To do alignment rounding, just do (addr & ALIGN_(alignment))
#define ALIGN_4K (~0xfffULL)
#define ALIGN_2M (~0x1fffffULL)
#define ALIGN_1G (~0x3fffffffULL)

#define PHYSOFF 0x20000000000ULL

#define phys2virt(addr) ((addr) + PHYSOFF)
#define virt2phys(addr) ((addr) - PHYSOFF)

/**
 * These are represented as the exponent for powers of two
 */
enum frame_size_t {
    FRAME_4K = 12,
    FRAME_2M = 21,
    FRAME_1G = 30,
};

#define ALIGN_FRAME(addr, frame_size) (addr & (~((1<<frame_size) - 1)))

void init_phys();

uintptr_t palloc(size_t size);
void* pvalloc(size_t size);