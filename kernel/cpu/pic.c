#include "pic.h"

#include <cpu/io.h>

#define PIC1         0x20
#define PIC1_COMMAND PIC1
#define PIC1_OFFSET  0x20
#define PIC1_DATA    (PIC1 + 1)

#define PIC2         0xA0
#define PIC2_COMMAND PIC2
#define PIC2_OFFSET  0x28
#define PIC2_DATA    (PIC2 + 1)

#define PIC_EOI      0x20

#define ICW1_ICW4    0x01
#define ICW1_INIT    0x10

void irq_ack(int n) {
    if (n >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap() {
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, PIC1_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);
    io_wait();

    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    outb(PIC1_DATA, 0x00);
    io_wait();
    outb(PIC2_DATA, 0x00);
    io_wait();
}