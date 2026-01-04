//
// Created by andy on 1/2/26.
//

#pragma once
#include <stdint.h>

void init_interrupts();

#define INT_DE 0
#define INT_DB 1
#define INT_NMI 2
#define INT_BP 3
#define INT_OF 4
#define INT_BR 5
#define INT_UD 6
#define INT_NM 7
#define INT_DF 8
#define INT_TS 10
#define INT_NP 11
#define INT_SS 12
#define INT_GP 13
#define INT_PF 14
#define INT_MF 16
#define INT_AC 17
#define INT_MC 18
#define INT_XM 19
#define INT_VE 20
#define INT_CP 21

#define IRQ0 32

typedef void (*isr_t)();
typedef unsigned long long int uword_t;

struct interrupt_frame {
    uword_t ip;
    uword_t cs;
    uword_t flags;
    uword_t sp;
    uword_t ss;
};

void set_isr(uint8_t index, isr_t handler);
void set_isr_trap(uint8_t index, isr_t handler);
