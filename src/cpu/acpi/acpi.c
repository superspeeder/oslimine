//
// Created by andy on 1/3/26.
//

#include <stdint.h>
#include <stddef.h>

#include "acpi.h"
#include "boot.h"
#include "cpu/mem/paging.h"
#include "drivers/serial.h"

// Definitions from https://wiki.osdev.org/RSDP
struct RSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

struct XSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress; // deprecated since version 2.0

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

struct RSDT_t {
    struct ACPISDTHeader h;
    uint32_t PointerToOtherSDT[];
};

struct XSDT_t {
    struct ACPISDTHeader h;
    uint64_t PointerToOtherSDT[];
};

static struct RSDP_t *rsdp;

static struct RSDT_t *rsdt;
static struct XSDT_t *xsdt;
static bool extended = false;

static const char RSDP_SIGNATURE[] = "RSD PTR ";

static bool validate_signature_rsdp(const char *signature) {
    for (int i = 0; i < 8; i++) {
        if (signature[i] != RSDP_SIGNATURE[i]) {
            return false;
        }
    }

    return true;
}

static bool validate_checksum(const void *data, const size_t size) {
    uint8_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += ((uint8_t *) data)[i];
    }

    return sum == 0;
}

static bool validate_sdt_checksum(struct ACPISDTHeader *tableHeader) {
    unsigned char sum = 0;

    for (uint32_t i = 0; i < tableHeader->Length; i++) {
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

static int init_from_table(struct ACPISDTHeader *header) {
    if (!validate_sdt_checksum(header)) {
        return EACPI_INVALID_CHECKSUM;
    }
    write_serial_string(SERIAL_COM1, "Reading ACPI Table: ");
    write_serial_string_n(SERIAL_COM1, header->Signature, 4);
    write_serial_string(SERIAL_COM1, "\r\n");
    return 0;
}

int init_acpi() {
    auto const rsdp_info = get_limine_rsdp();
    mmap_identity((uintptr_t)rsdp_info->address, sizeof(struct XSDP_t), nullptr);
    rsdp = (struct RSDP_t *) rsdp_info->address;

    if (!validate_signature_rsdp(rsdp->Signature)) {
        return EACPI_INVALID_SIGNATURE;
    }

    if (!validate_checksum(rsdp, sizeof(struct RSDP_t))) {
        return EACPI_INVALID_CHECKSUM;
    }

    if (rsdp->Revision >= 2) {
        // XSDP
        auto const xsdp = (struct XSDP_t *) rsdp;
        if (!validate_checksum(xsdp, sizeof(struct XSDP_t))) {
            return EACPI_INVALID_CHECKSUM;
        }

        extended = true;
        rsdt = nullptr;
        xsdt = (struct XSDT_t *) xsdp->XsdtAddress;
        validate_sdt_checksum(&xsdt->h);
    } else {
        extended = false;
        rsdt = (struct RSDT_t *) (uint64_t) rsdp->RsdtAddress;
        xsdt = nullptr;
        validate_sdt_checksum(&rsdt->h);
        const size_t num_entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
        for (size_t i = 0; i < num_entries; i++) {
            const int ec = init_from_table((struct ACPISDTHeader *) (uint64_t) rsdt->PointerToOtherSDT[i]);
            if (ec != 0) {
                return ec;
            }
        }
    }

    return 0;
}

void log_acpi_info() {
}
