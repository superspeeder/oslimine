//
// Created by andy on 1/2/26.
//

#include "ports.h"

void port_out_byte(uint16_t port, uint8_t data) {
    asm("out %%al, %%dx" : : "a" (data), "d" (port));
}

void port_out_word(uint16_t port, uint16_t data) {
    asm("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void port_out_dword(uint16_t port, uint32_t data) {
    asm("out %%eax, %%dx" : : "a" (data), "d" (port));
}

uint8_t port_in_byte(uint16_t port) {
    uint8_t result;
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

uint16_t port_in_word(uint16_t port) {
    uint16_t result;
    asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

uint32_t port_in_dword(uint16_t port) {
    uint32_t result;
    asm("in %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}
