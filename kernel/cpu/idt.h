#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct idt_entry_s {
    uint16_t offset0;
    uint16_t selector;
    uint8_t ist;
    uint8_t gate_type;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t zero;
} __attribute__((packed)) idt_entry_t;

struct regs {
    uintptr_t r15;
    uintptr_t r14;
    uintptr_t r13;
    uintptr_t r12;
    uintptr_t r11;
    uintptr_t r10;
    uintptr_t r9;
    uintptr_t r8;
    uintptr_t rbp;
    uintptr_t rdi;
    uintptr_t rsi;
    uintptr_t rdx;
    uintptr_t rcx;
    uintptr_t rbx;
    uintptr_t rax;
    uintptr_t int_no;
    uintptr_t err_code;
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
};

typedef int(*irq_handler_t)(struct regs*);

void irq_set_handler(size_t irq, irq_handler_t handler);
void idt_init();