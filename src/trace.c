#include "trace.h"

#include <stdint.h>
#include "drivers/serial.h"

int64_t tracestackdepth = 0;

void _trace(const char *text) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m│\033[0m   ");
    }
    write_serial_string(SERIAL_COM1, text);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void _trace2(const char *text, const char *text2) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m│\033[0m   ");
    }
    write_serial_string(SERIAL_COM1, text);
    write_serial_string(SERIAL_COM1, text2);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void _trace2e(const char *text, const char *text2) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth - 1; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m│\033[0m   ");
    }
    if (tracestackdepth > 0) {
        write_serial_string(SERIAL_COM1, "\033[2m├──\033[0m ");
    }

    write_serial_string(SERIAL_COM1, text);
    write_serial_string(SERIAL_COM1, text2);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void _traceaddr(const uintptr_t ptr) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m|\033[0m   ");
    }
    write_serial_string(SERIAL_COM1, "Addr: ");
    write_serial_hex(SERIAL_COM1, ptr);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void _traceint(const char *text, const uint64_t val) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m|\033[0m   ");
    }

    write_serial_string(SERIAL_COM1, text);
    write_serial_string(SERIAL_COM1, ": ");
    write_serial_int(SERIAL_COM1, val);
    write_serial_string(SERIAL_COM1, "\r\n");
}

void _traceinth(const char *text, const uint64_t val) {
    write_serial_string(SERIAL_COM1, "[TRACE] ");
    for (int64_t i = 0; i < tracestackdepth; i++) {
        write_serial_string(SERIAL_COM1, "\033[2m|\033[0m   ");
    }

    write_serial_string(SERIAL_COM1, text);
    write_serial_string(SERIAL_COM1, ": ");
    write_serial_hex(SERIAL_COM1, val);
    write_serial_string(SERIAL_COM1, "\r\n");
}
