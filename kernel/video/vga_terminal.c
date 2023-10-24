#include "vga_terminal.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

uint8_t terminal_color;
uint16_t* terminal_buffer;
uint8_t stack_terminal_buffer[VGA_WIDTH * VGA_HEIGHT];

terminal_t vga_terminal = {.init = vga_terminal_init, .putchar = vga_terminal_putchar, .clear = vga_terminal_clear_terminal, .rows = VGA_HEIGHT, .columns = VGA_HEIGHT};

static inline uint8_t terminal_vga_color(vga_color_t fg, vga_color_t bg) {
return fg | bg << 4;
}

void vga_terminal_init() {
    terminal_buffer = (uint16_t*) 0xB8000;
    terminal_color = terminal_vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_set_row(0);
    terminal_set_column(0);
}

void vga_terminal_set_color(uint8_t color) {
    terminal_color = color;
}

uint8_t vga_terminal_get_color() {
    return terminal_color;
}

static inline uint16_t terminal_vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) color << 8 | c;
}

void vga_terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    terminal_buffer[y * VGA_WIDTH + x] = terminal_vga_entry(c, color);
    stack_terminal_buffer[y * VGA_WIDTH + x] = c;
}

void vga_terminal_putchar(char c) {
    switch (c) {
        case '\n': {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_row = 0;
            }

            break;
        }

        default: {
            vga_terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
            if (++terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                if (++terminal_row == VGA_HEIGHT) {
                    terminal_row = 0;
                }
            }

            break;
        }
    }
}

void vga_terminal_clear_terminal() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_terminal_putentryat(' ', terminal_color, x, y);
        }
    }
}

uint8_t* vga_get_stack_buffer() {
    return stack_terminal_buffer;
}