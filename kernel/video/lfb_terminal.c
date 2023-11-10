#include "lfb_terminal.h"

#include <video/lfb.h>
#include <video/vga_terminal.h>
#include <lib/string.h>

terminal_t lfb_terminal = {.init = lfb_terminal_init, .putchar = lfb_terminal_putchar, .clear = lfb_terminal_clear};

static psf_font_t* font;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void lfb_copy_from_vga() {
    uint32_t row = terminal_row;
    uint32_t column = terminal_column;

    terminal_row = 0;
    terminal_column = 0;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            uint8_t character = vga_get_membuffer()[y * VGA_WIDTH + x];
            if (!character) {
                break;
            }

            lfb_terminal_putchar(character);
        }

        ++terminal_row;
        terminal_column = 0;
    }

    terminal_row = row;
    terminal_column = column;
}

void lfb_terminal_set_font(psf_font_t* f) {
    font = f;
}

void lfb_terminal_init() {
    lfb_terminal.columns = lfb_width / 8;
    lfb_terminal.rows = lfb_height / 16;
}

void lfb_terminal_putchar(char ch) {
    switch (ch) {
        case '\n': {
            terminal_column = 0;

            if (terminal_row + 1 >= lfb_terminal.rows) {
                uint32_t row_length = lfb_width * 16 * 4;
                memcpy(lfb_get_membuffer(), lfb_get_membuffer() + row_length, row_length * (lfb_terminal.rows - 1));
                memset(lfb_get_membuffer() + row_length * (lfb_terminal.rows - 1), 0, row_length);
                memcpy(linear_framebuffer, lfb_get_membuffer(), lfb_width * lfb_height * 4);
            } else {
                ++terminal_row;
            }

            return;
        }
    }

    char* face = (char*) font->buffer + (ch * font->header.character_size);

    for (size_t j = 0; j < 16; j++) {
        for (size_t i = 0; i < 8; i++) {
            uint32_t nx = (terminal_column * 8) + i;
            uint32_t ny = (terminal_row * 16) + j;

            uint32_t color = (*face & (0b10000000 >> i)) > 0
                             ? terminal_get_color() : 0x00000000;

            lfb_set_pixel(nx, ny, color);
        }

        face++;
    }

    if (++terminal_column >= lfb_terminal.columns) {
        terminal_putchar('\n');
    }
}

void lfb_terminal_clear() {
    for (size_t y = 0; y < lfb_height; y++) {
        for (size_t x = 0; x < lfb_width; x++) {
            lfb_set_pixel(x, y, 0);
        }
    }
}