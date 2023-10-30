#include "timer.h"

#include <sys/process.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <cpu/pic.h>

uint32_t tick;

static int timer_handler(struct regs* r) {
    ++tick;
    irq_ack(0);
    switch_task(1);
    return 1;
}

void timer_init(uint32_t frequency) {
    irq_set_handler(0, timer_handler);

    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);

    uint8_t lo = (uint8_t) (divisor & 0xFF);
    uint8_t hi = (uint8_t) ((divisor >> 8) & 0xFF);

    outb(0x40, lo);
    outb(0x40, hi);
}