#include "kbd.h"

#include <cpu/idt.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <stdint.h>

static const char _kbd_scs[] = {
        0x0, 0x0, '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=', 0x0, 0x0,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
        0x0, 0x0, 'a', 's',
        'd', 'f', 'g', 'h',
        'j', 'k', 'l', ';',
        '\'','`', 0x0, '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/', 0x0, '*',
        0x0, ' ', 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, '7',
        '8', '9', '-', '4',
        '5', '6', '+', '1',
        '2', '3', '0', '.'
};

static fs_node_t* _kbd_out = 0;
static uint8_t _kbd_caps_lock = 0;
static uint8_t _kbd_shift = 0;

static char _kbd_chr(int scancode) {
    switch (scancode) {
        case 0x1C: // Enter
            return '\n';

        case 0x2A: // LShift down
            _kbd_shift = 1;
            return 0;

        case 0xAA: // LShift up
            _kbd_shift = 0;
            return 0;

        case 0x3A: // Caps Lock
            _kbd_caps_lock ^= 1;
            return 0;

        case 0x0E:
            return '\b';

        default:
            if (scancode >= sizeof(_kbd_scs)) {
                return 0;
            }

            char c = _kbd_scs[scancode];
            uint8_t upper = _kbd_caps_lock ^ _kbd_shift;
            if (c) {
                return (char) (c - upper * 32);
            } else {
                return 0;
            }
    }
}

static int kbd_handler(struct regs* r) {
    uint8_t scancode = inb(0x60);
    char c = _kbd_chr(scancode);
    if (c) {
        char buf[1] = {c};
        write_fs(_kbd_out, 0, 1, buf);
    }
    irq_ack(1);
    return 1;
}

void kbd_init(fs_node_t* out) {
    _kbd_out = out;
    irq_set_handler(1, kbd_handler);
}
