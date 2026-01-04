//
// Created by andy on 1/2/26.
//

#pragma once

#include <stdint.h>

void port_out_byte(uint16_t port, uint8_t data);
void port_out_word(uint16_t port, uint16_t data);
void port_out_dword(uint16_t port, uint32_t data);

uint8_t port_in_byte(uint16_t port);
uint16_t port_in_word(uint16_t port);
uint32_t port_in_dword(uint16_t port);
