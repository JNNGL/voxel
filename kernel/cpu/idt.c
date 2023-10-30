#include "idt.h"

#include <lib/terminal.h>
#include <lib/kprintf.h>
#include <cpu/mmu.h>
#include <cpu/pic.h>

typedef struct regs*(*interrupt_handler_t)(struct regs*);

static idt_entry_t idt[256];
static struct {
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed)) idt_pointer;

static irq_handler_t irq_routines[64];

extern struct regs* _isr0(struct regs*);
extern struct regs* _isr1(struct regs*);
extern struct regs* _isr2(struct regs*);
extern struct regs* _isr3(struct regs*);
extern struct regs* _isr4(struct regs*);
extern struct regs* _isr5(struct regs*);
extern struct regs* _isr6(struct regs*);
extern struct regs* _isr7(struct regs*);
extern struct regs* _isr8(struct regs*);
extern struct regs* _isr9(struct regs*);
extern struct regs* _isr10(struct regs*);
extern struct regs* _isr11(struct regs*);
extern struct regs* _isr12(struct regs*);
extern struct regs* _isr13(struct regs*);
extern struct regs* _isr14(struct regs*);
extern struct regs* _isr15(struct regs*);
extern struct regs* _isr16(struct regs*);
extern struct regs* _isr17(struct regs*);
extern struct regs* _isr18(struct regs*);
extern struct regs* _isr19(struct regs*);
extern struct regs* _isr20(struct regs*);
extern struct regs* _isr21(struct regs*);
extern struct regs* _isr22(struct regs*);
extern struct regs* _isr23(struct regs*);
extern struct regs* _isr24(struct regs*);
extern struct regs* _isr25(struct regs*);
extern struct regs* _isr26(struct regs*);
extern struct regs* _isr27(struct regs*);
extern struct regs* _isr28(struct regs*);
extern struct regs* _isr29(struct regs*);
extern struct regs* _isr30(struct regs*);
extern struct regs* _isr31(struct regs*);
extern struct regs* _irq0(struct regs*);
extern struct regs* _irq1(struct regs*);
extern struct regs* _irq2(struct regs*);
extern struct regs* _irq3(struct regs*);
extern struct regs* _irq4(struct regs*);
extern struct regs* _irq5(struct regs*);
extern struct regs* _irq6(struct regs*);
extern struct regs* _irq7(struct regs*);
extern struct regs* _irq8(struct regs*);
extern struct regs* _irq9(struct regs*);
extern struct regs* _irq10(struct regs*);
extern struct regs* _irq11(struct regs*);
extern struct regs* _irq12(struct regs*);
extern struct regs* _irq13(struct regs*);
extern struct regs* _irq14(struct regs*);
extern struct regs* _irq15(struct regs*);
extern struct regs* _isr123(struct regs*);
extern struct regs* _isr124(struct regs*);
extern struct regs* _isr125(struct regs*);
extern struct regs* _isr126(struct regs*);
extern struct regs* _isr128(struct regs*);

void idt_set_gate(uint8_t n, interrupt_handler_t handler, uint16_t selector, uint8_t flags, int userspace) {
    uintptr_t base = (uintptr_t) handler;
    idt[n].offset0 = base & 0xFFFF;
    idt[n].offset1 = (base >> 16) & 0xFFFF;
    idt[n].offset2 = (base >> 32) & 0xFFFFFFFF;
    idt[n].selector = selector;
    idt[n].ist = 0;
    idt[n].zero = 0;
    idt[n].gate_type = flags | (userspace ? 0x60 : 0);
}

void idt_init() {
    idt_pointer.limit = sizeof(idt);
    idt_pointer.base = (uintptr_t) &idt;

    idt_set_gate(0, _isr0, 0x08, 0x8E, 0);
    idt_set_gate(1, _isr1, 0x08, 0x8E, 0);
    idt_set_gate(2, _isr2, 0x08, 0x8E, 0);
    idt_set_gate(3, _isr3, 0x08, 0x8E, 0);
    idt_set_gate(4, _isr4, 0x08, 0x8E, 0);
    idt_set_gate(5, _isr5, 0x08, 0x8E, 0);
    idt_set_gate(6, _isr6, 0x08, 0x8E, 0);
    idt_set_gate(7, _isr7, 0x08, 0x8E, 0);
    idt_set_gate(8, _isr8, 0x08, 0x8E, 0);
    idt_set_gate(9, _isr9, 0x08, 0x8E, 0);
    idt_set_gate(10, _isr10, 0x08, 0x8E, 0);
    idt_set_gate(11, _isr11, 0x08, 0x8E, 0);
    idt_set_gate(12, _isr12, 0x08, 0x8E, 0);
    idt_set_gate(13, _isr13, 0x08, 0x8E, 0);
    idt_set_gate(14, _isr14, 0x08, 0x8E, 0);
    idt_set_gate(15, _isr15, 0x08, 0x8E, 0);
    idt_set_gate(16, _isr16, 0x08, 0x8E, 0);
    idt_set_gate(17, _isr17, 0x08, 0x8E, 0);
    idt_set_gate(18, _isr18, 0x08, 0x8E, 0);
    idt_set_gate(19, _isr19, 0x08, 0x8E, 0);
    idt_set_gate(20, _isr20, 0x08, 0x8E, 0);
    idt_set_gate(21, _isr21, 0x08, 0x8E, 0);
    idt_set_gate(22, _isr22, 0x08, 0x8E, 0);
    idt_set_gate(23, _isr23, 0x08, 0x8E, 0);
    idt_set_gate(24, _isr24, 0x08, 0x8E, 0);
    idt_set_gate(25, _isr25, 0x08, 0x8E, 0);
    idt_set_gate(26, _isr26, 0x08, 0x8E, 0);
    idt_set_gate(27, _isr27, 0x08, 0x8E, 0);
    idt_set_gate(28, _isr28, 0x08, 0x8E, 0);
    idt_set_gate(29, _isr29, 0x08, 0x8E, 0);
    idt_set_gate(30, _isr30, 0x08, 0x8E, 0);
    idt_set_gate(31, _isr31, 0x08, 0x8E, 0);

    idt_set_gate(32, _irq0, 0x08, 0x8E, 0);
    idt_set_gate(33, _irq1, 0x08, 0x8E, 0);
    idt_set_gate(34, _irq2, 0x08, 0x8E, 0);
    idt_set_gate(35, _irq3, 0x08, 0x8E, 0);
    idt_set_gate(36, _irq4, 0x08, 0x8E, 0);
    idt_set_gate(37, _irq5, 0x08, 0x8E, 0);
    idt_set_gate(38, _irq6, 0x08, 0x8E, 0);
    idt_set_gate(39, _irq7, 0x08, 0x8E, 0);
    idt_set_gate(40, _irq8, 0x08, 0x8E, 0);
    idt_set_gate(41, _irq9, 0x08, 0x8E, 0);
    idt_set_gate(42, _irq10, 0x08, 0x8E, 0);
    idt_set_gate(43, _irq11, 0x08, 0x8E, 0);
    idt_set_gate(44, _irq12, 0x08, 0x8E, 0);
    idt_set_gate(45, _irq13, 0x08, 0x8E, 0);
    idt_set_gate(46, _irq14, 0x08, 0x8E, 0);
    idt_set_gate(47, _irq15, 0x08, 0x8E, 0);

    idt_set_gate(123, _isr123, 0x08, 0x8E, 0);
    idt_set_gate(124, _isr124, 0x08, 0x8E, 0);
    idt_set_gate(125, _isr125, 0x08, 0x8E, 0);
    idt_set_gate(126, _isr126, 0x08, 0x8E, 0);
    idt_set_gate(128, _isr128, 0x08, 0x8E, 1);

    asm volatile("lidt %0" : : "m"(idt_pointer));
}

void irq_set_handler(size_t irq, irq_handler_t handler) {
    for (size_t i = 0; i < 4; i++) {
        if (irq_routines[i * 16 + irq]) {
            continue;
        }

        irq_routines[i * 16 + irq] = handler;
        break;
    }
}

static void double_fault(struct regs* r) {
    puts("Double Fault.");
}

static void general_protection_fault(struct regs* r) {
    puts("General Protection Fault.");
}

static void page_fault(struct regs* r) {
    uintptr_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    if ((r->err_code & 0x03) == 0x03) {
        if (!mmu_copy_on_write(faulting_address)) {
            return;
        }
    }

    if (r->cs == 0x08) {
        kprintf("Page Fault (kernel) at %llx\n", faulting_address);
        while (1);
    } else {
        kprintf("User page fault at %llx\n", faulting_address); // TODO: SIGSEGV
    }
}

static struct regs* syscall(struct regs* r) {
    kprintf("syscall stub a=%llx b=%llx\n", r->rax, r->rbx);
    while (1);
    asm volatile("sti");
    return r;
}

static void exception(struct regs* r, const char* desc) {
    if (r->cs == 0x08) {
        puts(desc); // TODO
    }
}

static void handle_irq(struct regs* r, int irq) {
    for (size_t i = 0; i < 4; i++) {
        irq_handler_t handler = irq_routines[i * 16 + irq];
        if (!handler) {
            break;
        }

        if (handler(r)) {
            return;
        }
    }

    irq_ack(irq);
}

struct regs* isr_handler(struct regs* r) {
#define _exception(d) \
    exception(r, d);  \
    break;

#define _irq(i)            \
    handle_irq(r, i - 32); \
    break;

    switch (r->int_no) {
        case 0: _exception("divide_by_zero");
        case 3: _exception("breakpoint");
        case 4: _exception("overflow");
        case 5: _exception("bound range exceeded");
        case 6: _exception("invalid opcode");
        case 7: _exception("device not available");
        case 8: double_fault(r); break;
        case 10: _exception("invalid tss");
        case 11: _exception("segment not present");
        case 12: _exception("stack-segment fault");
        case 13: general_protection_fault(r); break;
        case 14: page_fault(r); break;
        case 16: _exception("floating point exception");
        case 17: _exception("alignment check");
        case 18: _exception("machine check");
        case 19: _exception("simd floating-point exception");
        case 20: _exception("virtualization exception");
        case 21: _exception("control protection exception");
        case 28: _exception("hypervisor injection exception");
        case 29: _exception("vmm communication exception");
        case 30: _exception("security exception");
        case 32: _irq(32);
        case 33: _irq(33);
        case 34: _irq(34);
        case 35: _irq(35);
        case 36: _irq(36);
        case 37: _irq(37);
        case 38: _irq(38);
        case 39: break;
        case 40: _irq(40);
        case 41: _irq(41);
        case 42: _irq(42);
        case 43: _irq(43);
        case 44: _irq(44);
        case 45: _irq(45);
        case 46: _irq(46);
        case 47: _irq(47);
        case 128: return syscall(r);
        default:
            puts("Unexpected interrupt"); // TODO
            break;
    }

    return r;
}