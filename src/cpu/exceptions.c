//
// Created by andy on 1/3/26.
//

#include "exceptions.h"

#include "drivers/serial.h"
#include "interrupts.h"

__attribute__((noreturn)) static void hcf() {
    while (1) {
        asm volatile("hlt");
    }
}

__attribute__((interrupt)) void page_fault_handler(const struct interrupt_frame *frame, const uword_t error_code) {
    (void)error_code;
    (void)frame;

    uint64_t cr2;
    asm volatile("mov %%cr2, %%rax" : "=a"(cr2) :);

    write_serial_string(SERIAL_COM1, "Page Fault!\r\nAddr: ");
    write_serial_hex(SERIAL_COM1, cr2);
    write_serial_string(SERIAL_COM1, "\r\n");
    hcf();
}

__attribute__((interrupt)) void gpf_handler(const struct interrupt_frame *frame, const uword_t error_code) {
    (void)error_code;
    (void)frame;

    write_serial_string(SERIAL_COM1, "General Protection Fault!\n");
    hcf();
}

__attribute__((interrupt)) void double_fault_handler(const struct interrupt_frame *frame, const uword_t error_code) {
    (void)error_code;
    (void)frame;

    write_serial_string(SERIAL_COM1, "Double Fault!\n");
    hcf();
}

__attribute__((interrupt)) void null_exception_handler(const struct interrupt_frame *frame, const uword_t error_code) {
    (void)error_code;
    (void)frame;
}

__attribute__((interrupt)) void null_isa_interrupt_handler(const struct interrupt_frame *frame) {
    (void)frame;
}

__attribute__((interrupt)) void nmi_handler(const struct interrupt_frame *frame) {
    (void)frame;
    write_serial_string(SERIAL_COM1, "NMI Exception!\n");
    hcf();
}

void load_exception_handlers() {
    set_isr(INT_DE, null_isa_interrupt_handler);
    set_isr_trap(INT_DB, null_isa_interrupt_handler);
    set_isr(INT_NMI, nmi_handler);
    set_isr_trap(INT_BP, null_isa_interrupt_handler);
    set_isr_trap(INT_OF, null_isa_interrupt_handler);
    set_isr(INT_BR, null_isa_interrupt_handler);
    set_isr(INT_UD, null_isa_interrupt_handler);
    set_isr(INT_NM, null_isa_interrupt_handler);
    set_isr(INT_DF, double_fault_handler);
    set_isr(INT_TS, null_exception_handler);
    set_isr(INT_NP, null_exception_handler);
    set_isr(INT_SS, null_exception_handler);
    set_isr(INT_GP, gpf_handler);
    set_isr(INT_PF, page_fault_handler);
    set_isr(INT_MF, null_isa_interrupt_handler);
    set_isr(INT_AC, null_exception_handler);
    set_isr(INT_MC, null_isa_interrupt_handler);
    set_isr(INT_XM, null_isa_interrupt_handler);
    set_isr(INT_VE, null_isa_interrupt_handler);
    set_isr(INT_CP, null_exception_handler);
}
