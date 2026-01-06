//
// Created by andy on 1/3/26.
//

#include <stddef.h>
#include <stdint.h>

#include "acpi.h"
#include "boot.h"
#include "trace.h"

#include "apic.h"
#include "fadt.h"

#define MKSIG(sig) ((uint32_t)((sig)[0]) | (((uint32_t)((sig)[1])) << 8) | (((uint32_t)((sig)[2])) << 16) | (((uint32_t)((sig)[3])) << 24))
#define MKSIG2(c0, c1, c2, c3) ((uint32_t)(c0) | (((uint32_t)(c1) << 8) | (((uint32_t)(c2)) << 16) | (((uint32_t)(c3)) << 24)))

#define SIG_FACP (MKSIG2('F', 'A', 'C', 'P'))
#define SIG_APIC (MKSIG2('A', 'P', 'I', 'C'))
#define SIG_HPET (MKSIG2('H', 'P', 'E', 'T'))

static struct rsdp_t *rsdp;

static rsdt_t *rsdt;
static xsdt_t *xsdt;
static bool    extended = false;

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
        sum += ((uint8_t *)data)[i];
    }

    return sum == 0;
}

static bool validate_sdt_checksum(acpi_sdt_header_t *tableHeader) {
    unsigned char sum = 0;

    for (uint32_t i = 0; i < tableHeader->length; i++) {
        sum += ((char *)tableHeader)[i];
    }

    return sum == 0;
}

static int init_from_table(acpi_sdt_header_t *header) {
    trace_enter();
    if (!validate_sdt_checksum(header)) {
        trace_error("EACPI_INVALID_CHECKSUM") trace_exit();
        return EACPI_INVALID_CHECKSUM;
    }
    char sig[5] = {header->signature[0], header->signature[1], header->signature[2], header->signature[3], 0};
    trace2("Reading ACPI Table: ", sig);

    uint32_t sign = MKSIG(sig);
    int ec;
    switch (sign) {
    case SIG_FACP:
        ec = acpi_init_from_fadt((fadt_t *)header);
        trace_exit();
        return ec;
    case SIG_APIC:
        ec = acpi_init_from_apic((madt_header_t *)header);
        trace_exit();
        return ec;
    // case SIG_HPET:
    //     return init_from_fadt(header);
    default:
        trace_exit();
        return 0;
    }
}

int init_acpi() {
    trace_enter();
    auto const rsdp_info = get_limine_rsdp();
    rsdp                 = (struct rsdp_t *)rsdp_info->address;
    trace_log_addr(rsdp_info->address);
    trace_log_addr(rsdp);

    if (!validate_signature_rsdp(rsdp->signature)) {
        trace_exit();
        return EACPI_INVALID_SIGNATURE;
    }

    if (!validate_checksum(rsdp, sizeof(struct rsdp_t))) {
        trace_exit();
        return EACPI_INVALID_CHECKSUM;
    }

    if (rsdp->revision >= 2) {
        // XSDP
        auto const xsdp = (struct xsdp_t *)rsdp;
        if (!validate_checksum(xsdp, sizeof(struct xsdp_t))) {
            trace_exit();
            return EACPI_INVALID_CHECKSUM;
        }

        extended = true;
        rsdt     = nullptr;
        xsdt     = (struct XSDT_t *)(xsdp->xsdt_address + get_limine_hhdm()->offset);
        validate_sdt_checksum(&xsdt->header);
    } else {
        extended = false;
        rsdt     = (struct RSDT_t *)(((uint64_t)rsdp->rsdt_address) + get_limine_hhdm()->offset);
        xsdt     = nullptr;
        validate_sdt_checksum(&rsdt->header);
        const size_t num_entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
        for (size_t i = 0; i < num_entries; i++) {
            const int ec = init_from_table((acpi_sdt_header_t *)(((uint64_t)rsdt->pointer_to_other_sdt[i]) + get_limine_hhdm()->offset));
            if (ec != 0) {
                trace_exit();
                return ec;
            }
        }
    }

    trace_exit();
    return 0;
}

void log_acpi_info() {}
