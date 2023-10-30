#pragma once

#include <stdint.h>

typedef uint64_t gdt_entry_t;

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access_byte;
    uint8_t flags;
    uint8_t base2;
    uint32_t base3;
    uint32_t reserved;
} gdt64_entry_t;

typedef struct {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

void gdt_encode_entry(gdt_entry_t* target, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags);
void tss_encode(gdt_entry_t* target, tss_entry_t* tss, void* stack);
void gdt_load(uint16_t limit, gdt_entry_t* base);
void set_kernel_stack(uintptr_t stack);