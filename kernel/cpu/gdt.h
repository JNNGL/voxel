#pragma once

#include <stdint.h>

typedef uint64_t gdt_entry_t;

void gdt_encode_entry(gdt_entry_t* target, uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags);
void gdt_load(uint16_t limit, gdt_entry_t* base);