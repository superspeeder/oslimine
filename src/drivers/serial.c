//
// Created by andy on 1/2/26.
//

#include "serial.h"

#include <stddef.h>

#include "cpu/ports.h"

int serial_init(const enum serial_port_id port) {
    port_out_byte(port + 1, 0x00);
    port_out_byte(port + 3, 0x80);
    port_out_byte(port + 0, 0x03);
    port_out_byte(port + 1, 0x00);
    port_out_byte(port + 3, 0x03);
    port_out_byte(port + 2, 0xc7);
    port_out_byte(port + 4, 0x0b);
    port_out_byte(port + 4, 0x1e);
    port_out_byte(port + 0, 0xae);
    if (port_in_byte(port + 0) != 0xae) {
        return ESERIAL_FAULTY;
    }
    port_out_byte(port + 4, 0x0f);
    return 0;
}

static int serial_recieved(const enum serial_port_id port) {
    return port_in_byte(port + 5) & 1;
}

static int is_transmit_empty(const enum serial_port_id port) {
    return port_in_byte(port + 5) & 0x20;
}

void write_serial(const enum serial_port_id port, const unsigned char data) {
    while (is_transmit_empty(port) == 0) {
    }
    port_out_byte(port, data);
}

unsigned char read_serial(const enum serial_port_id port) {
    while (serial_recieved(port) == 0) {
    }
    return port_in_byte(port);
}

void write_serial_string(const enum serial_port_id port, const char *str) {
    while (*str) {
        write_serial(port, *str);
        str++;
    }
}

void write_serial_string_n (const enum serial_port_id port, const char *str, const size_t len) {
    for (size_t i = 0; i < len; i++) {
        write_serial(port, str[i]);
    }
}

void write_serial_int(const enum serial_port_id port, const uint64_t num) {
    const uint64_t rem = num / 10;
    const uint64_t n = num % 10;
    if (rem != 0) {
        write_serial_int(port, rem);
    }
    write_serial(port, n + '0');
}

static unsigned char hexdig[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void write_serial_hex(const enum serial_port_id port, const uint64_t num) {
    const uint64_t rem = num >> 4;
    const uint64_t n = num & 0xF;
    if (rem != 0) {
        write_serial_hex(port, rem);
    }
    write_serial(port, hexdig[n]);
}

void write_serial_hex_pad(const enum serial_port_id port, uint64_t num, const uint8_t pad) {
    if ((num & ((1ULL << (pad * 4)) - 1)) != num) {
        write_serial_hex(port, num); // passes padding
    }
    for (uint8_t i = 0; i < pad; i++) {
        uint64_t dig = (num >> (i * 4)) & 0xF;
        write_serial(port, hexdig[dig]);
    }
}
