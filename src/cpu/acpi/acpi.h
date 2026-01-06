//
// Created by andy on 1/3/26.
//

#pragma once

#include <stdint.h>

int init_acpi();

void log_acpi_info();

#define EACPI_INVALID_SIGNATURE 1
#define EACPI_INVALID_CHECKSUM 2

typedef struct rsdp_t {
    char     signature[8];
    uint8_t  checksum;
    char     oemid[6];
    uint8_t  revision;
    uint32_t rsdt_address;
} __attribute__((packed)) rsdp_t;

typedef struct xsdp_t {
    char     signature[8];
    uint8_t  checksum;
    char     oemid[6];
    uint8_t  revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__((packed)) xsdp_t;

typedef struct acpi_sdt_header_t {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;

typedef struct generic_address_structure_t {
    uint8_t  address_space;
    uint8_t  bit_width;
    uint8_t  bit_offset;
    uint8_t  access_size;
    uint64_t address;
} generic_address_structure_t;

typedef struct fadt_t {
    acpi_sdt_header_t header;
    uint32_t          firmware_control;
    uint32_t          dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t _reserved;

    uint8_t  preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_required;
    uint8_t  pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t  pm1_event_length;
    uint8_t  pm1_control_length;
    uint8_t  pm2_control_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_length;
    uint8_t  gpe1_length;
    uint8_t  gpe1_base;
    uint8_t  cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t boot_architecture_flags;

    uint8_t  _reserved2;
    uint32_t flags;

    // 12 byte structure; see below for details
    generic_address_structure_t reset_reg;

    uint8_t reset_value;
    uint8_t _reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    generic_address_structure_t x_pm1a_event_block;
    generic_address_structure_t x_pm1b_event_block;
    generic_address_structure_t x_pm1a_control_block;
    generic_address_structure_t x_pm1b_control_block;
    generic_address_structure_t x_pm2_control_blokc;
    generic_address_structure_t x_pm_timer_block;
    generic_address_structure_t x_gpe0_block;
    generic_address_structure_t x_gpe1_block;
} fadt_t;

typedef struct rsdt_t {
    acpi_sdt_header_t header;
    uint32_t          pointer_to_other_sdt[];
} rsdt_t;

typedef struct xsdt_t {
    acpi_sdt_header_t header;
    uint64_t          pointer_to_other_sdt[];
} xsdt_t;

typedef struct madt_header_t {
    acpi_sdt_header_t header;
    uint32_t          local_apic_address;
    uint32_t          flags;
} __attribute__((packed)) madt_header_t;

typedef struct madt_entry_header_t {
    uint8_t type;
    uint8_t record_length;
} __attribute__((packed)) madt_entry_header_t;

typedef struct madt_entry_processor_local_apic_t {
    madt_entry_header_t header;
    uint8_t             acpi_processor_id;
    uint8_t             apic_id;
    uint32_t            flags;
} __attribute__((packed)) madt_entry_processor_local_apic_t;

typedef struct madt_entry_processor_io_apic_t {
    madt_entry_header_t header;
    uint8_t             io_apic_id;
    uint8_t             _reserved;
    uint32_t            io_apic_address;
    uint32_t            global_system_interrupt_base;
} __attribute__((packed)) madt_entry_io_apic_t;

typedef struct madt_entry_io_apic_non_maskable_interrupt_source_t {
    madt_entry_header_t header;
    uint8_t             nmi_source;
    uint8_t             _reserved;
    uint16_t            flags;
    uint32_t            global_system_interrupt;
} __attribute__((packed)) madt_entry_io_apic_non_maskable_interrupt_source_t;

typedef struct madt_entry_local_apic_non_maskable_interrupts_t {
    madt_entry_header_t header;
    uint8_t             acpi_processor_id;
    uint16_t            flags;
    uint8_t             lint;
} __attribute__((packed)) madt_entry_local_apic_non_maskable_interrupts_t;

typedef struct madt_entry_local_apic_address_override_t {
    madt_entry_header_t header;
    uint16_t            _reserved;
    uint64_t            address;
} __attribute__((packed)) madt_entry_local_apic_address_override_t;

typedef struct madt_entry_processor_local_x2apic_t {
    madt_entry_header_t header;
    uint16_t            _reserved;
    uint32_t            processor_local_x2apic_id;
    uint32_t            flags;
    uint32_t            acpi_id;
} __attribute__((packed)) madt_entry_processor_local_x2apic_t;

typedef struct hpet_t {
    acpi_sdt_header_t           header;
    uint8_t                     hardware_rev_id;
    uint8_t                     comparator_count : 5;
    uint8_t                     counter_size : 1;
    uint8_t                     reserved : 1;
    uint8_t                     legacy_replacement : 1;
    uint16_t                    pci_vendor_id;
    generic_address_structure_t address;
    uint8_t                     hpet_number;
    uint16_t                    minimum_tick;
    uint8_t                     page_protection;
} __attribute__((packed)) hpet_t;
