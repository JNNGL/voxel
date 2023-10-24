#pragma once

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

void insl(uint16_t port, uint32_t* buffer, uint32_t count);

void io_wait();

static inline void mmio_write8(void* ptr, uint8_t data) {
    *(volatile uint8_t*) ptr = data;
}

static inline uint8_t mmio_read8(void* ptr) {
    return *(volatile uint8_t*) ptr;
}

static inline void mmio_write16(void* ptr, uint16_t data) {
    *(volatile uint16_t*) ptr = data;
}

static inline uint8_t mmio_read16(void* ptr) {
    return *(volatile uint16_t*) ptr;
}

static inline void mmio_write32(void* ptr, uint32_t data) {
    *(volatile uint32_t*) ptr = data;
}

static inline uint8_t mmio_read32(void* ptr) {
    return *(volatile uint32_t*) ptr;
}

static inline void mmio_write64(void* ptr, uint64_t data) {
    *(volatile uint64_t*) ptr = data;
}

static inline uint8_t mmio_read64(void* ptr) {
    return *(volatile uint64_t*) ptr;
}