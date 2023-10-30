#include "gdt.h"

volatile tss_entry_t* current_tss;

void gdt_encode_entry(gdt_entry_t* entry, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags) {
    uint8_t* target = (uint8_t*) entry;
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] = (limit >> 16) & 0x0F;
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;
    target[5] = access_byte;
    target[6] |= (flags << 4);
}

void tss_encode(gdt_entry_t* target, tss_entry_t* tss, void* stack) {
    current_tss = tss; // TODO: set current tss on gdt load?
    gdt64_entry_t* entry64 = (gdt64_entry_t*) target;
    uintptr_t address = (uintptr_t) tss;
    entry64->limit0 = sizeof(*tss);
    entry64->base0 = address & 0xFFFF;
    entry64->base1 = (address >> 16) & 0xFF;
    entry64->base2 = (address >> 24) & 0xFF;
    entry64->base3 = (address >> 32) & 0xFFFFFFFF;
    entry64->access_byte = 0xE9;
    entry64->flags = 0;
    tss->rsp[0] = (uintptr_t) stack;
}

void set_kernel_stack(uintptr_t stack) {
    current_tss->rsp[0] = stack;
}