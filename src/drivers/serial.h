//
// Created by andy on 1/2/26.
//
#pragma once

#include <stdint.h>
#include <stddef.h>

enum serial_port_id {
    SERIAL_COM1 = 0x3f8,
    SERIAL_COM2 = 0x2f8,
    SERIAL_COM3 = 0x3e8,
    SERIAL_COM4 = 0x2e8,
    SERIAL_COM5 = 0x5f8,
    SERIAL_COM6 = 0x4f8,
    SERIAL_COM7 = 0x5e8,
    SERIAL_COM8 = 0x4e8,
};

#define ESERIAL_FAULTY 1

/**
 * Initialize a serial port with default settings (8n1, 38400 baud)
 *
 * @param port Port ID
 * @return Returns 0 if serial port was successfully initialized, ESERIAL_FAULTY if port is faulty.
 */
int serial_init(enum serial_port_id port);

void write_serial(enum serial_port_id port, unsigned char data);
unsigned char read_serial(enum serial_port_id port);

void write_serial_string(enum serial_port_id port, const char* str);
void write_serial_string_n(enum serial_port_id port, const char* str, size_t len);
void write_serial_int(enum serial_port_id port, uint64_t num);
void write_serial_hex(enum serial_port_id port, uint64_t num);
void write_serial_hex_pad(enum serial_port_id port, uint64_t num, uint8_t pad);
