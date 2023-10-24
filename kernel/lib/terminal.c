#include "terminal.h"

#include <lib/string.h>

size_t terminal_row;
size_t terminal_column;
uint32_t text_color;
size_t user_input_num;

terminal_t terminal;

void terminal_init() {
    user_input_num = 0;
    text_color = 0xFFFFFFFF;
    terminal.init();
}

void terminal_set_row(size_t row) {
    terminal_row = row;
}

void terminal_set_column(size_t column) {
    terminal_column = column;
}

void terminal_set_color(uint32_t newcolor) {
    text_color = newcolor;
}

size_t terminal_get_row() {
    return terminal_row;
}

size_t terminal_get_column() {
    return terminal_column;
}

uint32_t terminal_get_color() {
    return text_color;
}

void terminal_clear_terminal() {
    terminal.clear();
}

static void terminal_put_user_char(char c) {
    ++user_input_num;
    terminal.putchar(c);
}

void terminal_putchar(char c) {
    user_input_num = 0;
    terminal.putchar(c);
}

void terminal_put(const char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_putstring(const char* str) {
    terminal_put(str, strlen(str));
}

size_t terminal_get_columns() {
    return terminal.columns;
}

size_t terminal_get_rows() {
    return terminal.rows;
}

void terminal_process_input(char c) {
    if (c == '\b') {
        if (user_input_num) {
            size_t column = terminal_get_column();
            size_t row = terminal_get_row();
            if (--column == (size_t) -1) {
                column = terminal_get_columns() - 1;
                if (--row == (size_t) -1) {
                    row = terminal_get_rows() - 1;
                }
            }

            terminal_set_column(column);
            terminal_set_row(row);
            terminal_put_user_char(' ');
            terminal_set_column(column);
            user_input_num -= 2;
        } else {
            return;
        }
    } else {
        terminal_put_user_char(c);
    }
}

int putchar(int c) {
    terminal_putchar((char) c);
    return c;
}

int puts(const char* str) {
    terminal_putstring(str);
    terminal_putchar('\n');
    return 0;
}