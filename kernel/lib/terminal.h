#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct terminal_s {
    void(*putchar)(char c);
    void(*init)();
    void(*clear)();
    size_t rows;
    size_t columns;
} terminal_t;

extern terminal_t terminal;

extern size_t terminal_row;
extern size_t terminal_column;

void terminal_init();
void terminal_set_row(size_t row);
void terminal_set_column(size_t column);
void terminal_set_color(uint32_t color);
size_t terminal_get_row();
size_t terminal_get_column();
uint32_t terminal_get_color();
void terminal_clear_terminal();
void terminal_putchar(char c);
void terminal_put(const char* str, size_t size);
void terminal_putstring(const char* str);
size_t terminal_get_columns();
size_t terminal_get_rows();
void terminal_process_input(char c);

int putchar(int c);
int puts(const char* str);