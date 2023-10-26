#include "kprintf.h"

#include <lib/string.h>
#include <lib/ctype.h>

typedef uint32_t wint_t;

char* __int_str(intmax_t i, char b[], int base, uint8_t plus_sign_if_needed, uint8_t space_sign_if_needed,
                int padding_no, uint8_t justify, uint8_t zero_pad) {
    char digit[32] = {0};
    memset(digit, 0, 32);
    strcpy(digit, "0123456789");

    if (base == 16) {
        strcat(digit, "ABCDEF");
    } else if (base == 17) {
        strcat(digit, "abcdef");
        base = 16;
    }

    char* p = b;
    if (i < 0) {
        *p++ = '-';
        i *= -1;
    } else if (plus_sign_if_needed) {
        *p++ = '+';
    } else if (!plus_sign_if_needed && space_sign_if_needed) {
        *p++ = ' ';
    }

    intmax_t shifter = i;
    do {
        ++p;
        shifter = shifter / base;
    } while (shifter);

    *p = '\0';
    do {
        *--p = digit[i % base];
        i = i / base;

    } while (i);

    int padding = padding_no - (int) strlen(b);
    if (padding < 0) {
        padding = 0;
    }

    if (justify) {
        while (padding--) {
            if (zero_pad) {
                b[strlen(b)] = '0';
            } else {
                b[strlen(b)] = ' ';
            }
        }
    } else {
        char a[256] = {0};
        while (padding--) {
            if (zero_pad) {
                a[strlen(a)] = '0';
            } else {
                a[strlen(a)] = ' ';
            }
        }

        strcat(a, b);
        strcpy(b, a);
    }

    return b;
}

int kvprintf(const char* format, va_list list) {
    int chars = 0;
    char int_str_buffer[256] = {0};

    for (int i = 0; format[i]; ++i) {
        char specifier = '\0';
        char length = '\0';
        int length_spec = 0;
        int prec_spec = 0;
        uint8_t left_justify = 0;
        uint8_t zero_pad = 0;
        uint8_t space_no_sign = 0;
        uint8_t alt_form = 0;
        uint8_t plus_sign = 0;
        // uint8_t emode = 0;
        int expo = 0;

        if (format[i] == '%') {
            ++i;

            uint8_t ext_break = 0;
            while (1) {
                switch (format[i]) {
                    case '-':
                        left_justify = 1;
                        ++i;
                        break;

                    case '+':
                        plus_sign = 1;
                        ++i;
                        break;

                    case '#':
                        alt_form = 1;
                        ++i;
                        break;

                    case ' ':
                        space_no_sign = 1;
                        ++i;
                        break;

                    case '0':
                        zero_pad = 1;
                        ++i;
                        break;

                    default:
                        ext_break = 1;
                        break;
                }

                if (ext_break) {
                    break;
                }
            }

            while (isdigit(format[i])) {
                length_spec *= 10;
                length_spec += format[i] - 48;
                ++i;
            }

            if (format[i] == '*') {
                length_spec = va_arg(list, int);
                ++i;
            }

            if (format[i] == '.') {
                ++i;
                while (isdigit(format[i])) {
                    prec_spec *= 10;
                    prec_spec += format[i] - 48;
                    ++i;
                }

                if (format[i] == '*') {
                    prec_spec = va_arg(list, int);
                    ++i;
                }
            } else {
                prec_spec = 6;
            }

            if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
                format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
                length = format[i];
                ++i;
                if (format[i] == 'h') {
                    length = 'H';
                } else if (format[i] == 'l') {
                    length = 'q';
                    ++i;
                }
            }

            specifier = format[i];

            memset(int_str_buffer, 0, 256);

            int base = 10;
            if (specifier == 'o') {
                base = 8;
                specifier = 'u';
                if (alt_form) {
                    terminal_putstring("0");
                    ++chars;
                }
            }
            if (specifier == 'p') {
                base = 16;
                length = 'z';
                specifier = 'u';
            }

            switch (specifier) {
                case 'X':
                    base = 16;
                            __attribute__((fallthrough));

                case 'x':
                    base = base == 10 ? 17 : base;
                    if (alt_form) {
                        terminal_putstring("0x");
                        chars += 2;
                    }
                            __attribute__((fallthrough));

                case 'u': {
                    switch (length) {
                        case 0: {
                            uint32_t integer = va_arg(list, unsigned int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'H': {
                            uint8_t integer = (uint8_t) va_arg(list, unsigned int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'h': {
                            uint16_t integer = va_arg(list, unsigned int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'l': {
                            unsigned long integer = va_arg(list, unsigned long);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'q': {
                            uint64_t integer = va_arg(list, unsigned long long);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'j': {
                            uintmax_t integer = va_arg(list, uintmax_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'z': {
                            size_t integer = va_arg(list, size_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 't': {
                            ptrdiff_t integer = va_arg(list, ptrdiff_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        default:
                            break;
                    }

                    break;
                }

                case 'd':
                case 'i': {
                    switch (length) {
                        case 0: {
                            int integer = va_arg(list, int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'H': {
                            int8_t integer = (int8_t) va_arg(list, int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'h': {
                            short integer = va_arg(list, int);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'l': {
                            long integer = va_arg(list, long);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'q': {
                            long long integer = va_arg(list, long long);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'j': {
                            intmax_t integer = va_arg(list, intmax_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 'z': {
                            size_t integer = va_arg(list, size_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        case 't': {
                            ptrdiff_t integer = va_arg(list, ptrdiff_t);
                            __int_str(integer, int_str_buffer, base, plus_sign, space_no_sign, length_spec, left_justify, zero_pad);
                            terminal_putstring(int_str_buffer);
                            chars += strlen(int_str_buffer);
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }

                case 'c': {
                    if (length == 'l') {
                        terminal_putchar((char) va_arg(list, wint_t));
                    } else {
                        terminal_putchar((char) va_arg(list, int));
                    }

                    ++chars;
                    break;
                }

                case 's': {
                    char* str = va_arg(list, char*);
                    terminal_putstring(str);
                    chars += strlen(str);
                    break;
                }

                case 'n': {
                    switch (length) {
                        case 'H':
                            *(va_arg(list, int8_t*)) = chars;
                            break;

                        case 'h':
                            *(va_arg(list, short*)) = chars;
                            break;

                        case 0: {
                            int* a = va_arg(list, int*);
                            *a = chars;
                            break;
                        }

                        case 'l':
                            *(va_arg(list, long*)) = chars;
                            break;

                        case 'q':
                            *(va_arg(list, long long*)) = chars;
                            break;

                        case 'j':
                            *(va_arg(list, intmax_t*)) = chars;
                            break;

                        case 'z':
                            *(va_arg(list, size_t*)) = chars;
                            break;

                        case 't':
                            *(va_arg(list, ptrdiff_t*)) = chars;
                            break;

                        default:
                            break;
                    }
                    break;
                }

                case 'e':
                case 'E':
                    // emode = 1;

                case 'f':
                case 'F':
                case 'g':
                case 'G':
                    break;

                case 'a':
                case 'A':
                    break;

                default:
                    break;
            }

            if (specifier == 'e') {
                terminal_putstring("e+");
                chars += 2;
            } else if (specifier == 'E') {
                terminal_putstring("E+");
                chars += 2;
            }

            if (specifier == 'e' || specifier == 'E') {
                __int_str(expo, int_str_buffer, 10, 0, 0, 2, 0, 1);
                terminal_putstring(int_str_buffer);
                chars += strlen(int_str_buffer);
            }

        } else {
            terminal_putchar(format[i]);
            ++chars;
        }
    }

    return chars;
}

__attribute__((format(printf, 1, 2)))
int kprintf(const char* format, ...) {
    va_list list;
    va_start(list, format);
    int i = kvprintf(format, list);
    va_end(list);
    return i;
}