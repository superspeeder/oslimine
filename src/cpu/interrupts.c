//
// Created by andy on 1/2/26.
//


#include "interrupts.h"
#include "gdt.h"
#include "exceptions.h"

struct __attribute__((packed)) idt_t {
    uint16_t size;
    uint64_t address;
};

struct __attribute__((packed)) idt_entry_t {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

#define NUM_IDT_ENTRIES 256

static struct idt_entry_t idt[NUM_IDT_ENTRIES];
static struct idt_t idtr;


void set_isr(const uint8_t index, const isr_t handler) {
    const uint64_t addr = (uint64_t) handler;

    idt[index].selector = GDT_KERNEL_CODE;
    idt[index].ist = 0;
    idt[index].type_attr = 0x8E;
    idt[index].offset_low = (uint16_t) (addr & 0xFFFF);
    idt[index].offset_mid = (uint16_t) (addr >> 16) & 0xFFFF;
    idt[index].offset_high = (uint32_t) (addr >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void set_isr_trap(const uint8_t index, const isr_t handler) {
    const uint64_t addr = (uint64_t) handler;

    idt[index].selector = GDT_KERNEL_CODE;
    idt[index].ist = 0;
    idt[index].type_attr = 0x8F;
    idt[index].offset_low = (uint16_t) (addr & 0xFFFF);
    idt[index].offset_mid = (uint16_t) (addr >> 16) & 0xFFFF;
    idt[index].offset_high = (uint32_t) (addr >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void init_interrupts() {
    idtr.size = NUM_IDT_ENTRIES * sizeof(struct idt_entry_t) - 1;
    idtr.address = (uint64_t) &idt[0];

    load_exception_handlers();

    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
}
