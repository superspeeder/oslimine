#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "boot.h"
#include "cpu/gdt.h"
#include "cpu/mem/phys.h"
#include "drivers/serial.h"
#include "cpu/interrupts.h"
#include "cpu/acpi/acpi.h"
#include "cpu/mem/paging.h"


// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void log_memmap() {
    struct limine_memmap_response *memmap_response = get_limine_memmap();

    write_serial_string(SERIAL_COM1, "\033[1mMemmap entries:\033[0m\r\n");
    for (unsigned int i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        write_serial_string(SERIAL_COM1, "Base: ");
        write_serial_hex(SERIAL_COM1, entry->base);
        write_serial_string(SERIAL_COM1, ",\033[24GLength: ");
        write_serial_hex(SERIAL_COM1, entry->length);
        write_serial_string(SERIAL_COM1, ",\033[48GType: ");
        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                write_serial_string(SERIAL_COM1, "Usable");
                break;
            case LIMINE_MEMMAP_RESERVED:
                write_serial_string(SERIAL_COM1, "Reserved");
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                write_serial_string(SERIAL_COM1, "ACPI Reclaimable");
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                write_serial_string(SERIAL_COM1, "ACPI NVS");
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                write_serial_string(SERIAL_COM1, "Bad memory");
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                write_serial_string(SERIAL_COM1, "Bootloader reclaimable");
                break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                write_serial_string(SERIAL_COM1, "Usable");
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                write_serial_string(SERIAL_COM1, "Framebuffer");
                break;
            case LIMINE_MEMMAP_ACPI_TABLES:
                write_serial_string(SERIAL_COM1, "ACPI Tables");
                break;
            default: break;
        }
        write_serial_string(SERIAL_COM1, "\r\n");
    }
}

void draw_test_image() {
    const struct limine_framebuffer_response *framebuffer_response = get_limine_framebuffer();
    const struct limine_framebuffer *framebuffer = framebuffer_response->framebuffers[0];

    for (size_t i = 0; i < 100; i++) {
        volatile uint32_t *fb_ptr = framebuffer->address;
        fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff;
    }
}

void log_executable_info() {
    auto const executable_address = get_limine_executable_address();
    write_serial_string(SERIAL_COM1, "\033[1mExecutable Address Info:\033[0m\r\nPhysical Base: ");
    write_serial_hex(SERIAL_COM1, executable_address->physical_base);
    write_serial_string(SERIAL_COM1, "\r\nVirtual Base: ");
    write_serial_hex(SERIAL_COM1, executable_address->virtual_base);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void log_date_at_boot() {
    auto const date_at_boot = get_limine_date_at_boot();
    write_serial_string(SERIAL_COM1, "\033[1mDate at boot:\033[0m ");
    write_serial_int(SERIAL_COM1, date_at_boot->timestamp);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void kmain(void) {
    if (!verify_limine_base_revision()) {
        hcf();
    }

    init_gdt();
    serial_init(SERIAL_COM1);
    init_interrupts();
    log_executable_info();

    init_phys();
    init_paging();

    void* ptr = palloc(1ULL<<32);
    write_serial_string(SERIAL_COM1, "Allocated Phys Addr: ");
    write_serial_hex(SERIAL_COM1, (uintptr_t)ptr);
    write_serial_string(SERIAL_COM1, "\r\n");

    // write_serial_string(SERIAL_COM1, "\033[1mInitializing ACPI:\033[0m\r\n");
    // init_acpi();

    log_memmap();
    log_date_at_boot();

    draw_test_image();

    write_serial_string(SERIAL_COM1, "Hello, World!\n");

    hcf();
}
