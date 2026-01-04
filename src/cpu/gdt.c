//
// Created by andy on 1/3/26.
//

#include "gdt.h"

#include <stdint.h>

struct __attribute__((packed)) gdt_t {
    uint16_t size;
    uint64_t address;
};

struct __attribute__((packed)) long_gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags;
    uint8_t base_mid2;
    uint32_t base_high;
    uint32_t reserved;
};

struct __attribute__((packed)) gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
};

// TODO: TSS segment

#define NUM_GDT_ENTRIES 5

static struct gdt_entry_t gdt[NUM_GDT_ENTRIES];
static struct gdt_t gdtr;

static void set_gdt_entry(const uint8_t index, const uint32_t base, const uint32_t limit, const uint8_t access,
                          const uint8_t flags) {
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].base_mid = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].access = access;
    gdt[index].flags = (flags << 4) | ((limit >> 16) & 0b1111);
}

void init_gdt() {
    set_gdt_entry(0, 0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xfffff, 0x9a, 0xa);
    set_gdt_entry(2, 0, 0xfffff, 0x92, 0xc);
    set_gdt_entry(3, 0, 0xfffff, 0xfa, 0xa);
    set_gdt_entry(4, 0, 0xfffff, 0xf2, 0xc);

    gdtr.address = (uint64_t) &gdt;
    gdtr.size = NUM_GDT_ENTRIES * sizeof(struct gdt_entry_t) - 1;

    asm volatile ("lgdt %0" : : "m"(gdtr));
    asm volatile (
        "mov $0x10, %ax \n\t"
        "mov %ax, %ds \n\t"
        "mov %ax, %es \n\t"
        "mov %ax, %fs \n\t"
        "mov %ax, %gs \n\t"
        "mov %ax, %ss \n\t"
        "\n\t"
        "pop %rdi \n\t"
        "push $0x8 \n\t"
        "push %rdi \n\t"
        "lretq \n\t"
    );
}
