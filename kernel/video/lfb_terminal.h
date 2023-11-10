#pragma once

#pragma once

#include <lib/terminal.h>
#include <video/psf_font.h>

extern terminal_t lfb_terminal;

void lfb_copy_from_vga();
void lfb_terminal_set_font(psf_font_t* f);
void lfb_terminal_init();
void lfb_terminal_putchar(char c);
void lfb_terminal_clear();