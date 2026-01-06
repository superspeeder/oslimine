#include "fadt.h"
#include "trace.h"

static fadt_t *fadt_table = nullptr;

int acpi_init_from_fadt(fadt_t *fadt) {
    trace_enter();
    fadt_table = fadt;
    trace_exit();
    return 0;
}
