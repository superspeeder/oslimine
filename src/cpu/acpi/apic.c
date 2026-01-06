#include "apic.h"
#include "cpu/acpi/acpi.h"

static uint32_t local_apic_address;

int acpi_init_from_apic(madt_header_t* madt) {
    local_apic_address = madt->local_apic_address;

    uintptr_t end_addr = ((uintptr_t)madt) + madt->header.length;
    madt_entry_header_t* entry = (madt_entry_header_t*)((void*)madt + sizeof(madt_header_t));
    while ((uintptr_t)entry < end_addr) {
        
        entry = (madt_entry_header_t*)((void*)entry + entry->record_length);
    }
    return 0;
}
