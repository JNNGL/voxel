#include <stdio.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

struct _FILE {
    int fd;
    char* read_buf;
    int available;
    int offset;
    int read_from;
    int ungetc;
    int eof;
    int bufsiz;
    long last_read_start;
    char* name;
    char* write_buf;
    size_t written;
    size_t wbufsiz;
    struct _FILE* prev;
    struct _FILE* next;
};

FILE _stdin = {
        .fd = 0,
        .read_buf = NULL,
        .available = 0,
        .offset = 0,
        .read_from = 0,
        .ungetc = -1,
        .eof = 0,
        .last_read_start = 0,
        .bufsiz = BUFSIZ,
        .wbufsiz = BUFSIZ,
        .write_buf = NULL,
        .written = 0,
};

FILE _stdout = {
        .fd = 1,
        .read_buf = NULL,
        .available = 0,
        .offset = 0,
        .read_from = 0,
        .ungetc = -1,
        .eof = 0,
        .last_read_start = 0,
        .bufsiz = BUFSIZ,
        .wbufsiz = BUFSIZ,
        .write_buf = NULL,
        .written = 0,
};

FILE _stderr = {
        .fd = 2,
        .read_buf = NULL,
        .available = 0,
        .offset = 0,
        .read_from = 0,
        .ungetc = -1,
        .eof = 0,
        .last_read_start = 0,
        .bufsiz = BUFSIZ,
        .wbufsiz = BUFSIZ,
        .write_buf = NULL,
        .written = 0,
};

FILE* stdin = &_stdin;
FILE* stdout = &_stdout;
FILE* stderr = &_stderr;

struct _FILE* _head = 0;

void __stdio_init() {
    _stdin.read_buf = malloc(BUFSIZ);
    _stdin.name = strdup("stdin");
    _stdout.write_buf = malloc(BUFSIZ);
    _stdout.name = strdup("stdin");
    _stderr.write_buf = malloc(BUFSIZ);
    _stderr.name = strdup("stdin");
}

void __stdio_free() {
    if (stdout) {
        fflush(stdout);
    }
    if (stderr) {
        fflush(stderr);
    }
    while (_head) {
        fclose(_head);
    }
}

static size_t _read_bytes(FILE* f, char* out, size_t len) {
    size_t ret = 0;
    while (len > 0) {
        if (f->ungetc >= 0) {
            *out = f->ungetc;
            --len;
            ++out;
            ++ret;
            f->ungetc = -1;
            continue;
        }
        if (!f->available) {
            if (f->offset == f->bufsiz) {
                f->offset = 0;
            }
            f->last_read_start = syscall_seek(f->fd, 0, SEEK_CUR);
            ssize_t r = read(f->fd, &f->read_buf[f->offset], f->bufsiz - f->offset);
            if (r < 0) {
                return ret;
            } else {
                f->read_from = f->offset;
                f->available = r;
                f->offset += f->available;
            }
        }
        if (!f->available) {
            f->eof = 1;
            return ret;
        }
        while (f->read_from < f->offset && len > 0 && f->available > 0) {
            *out = f->read_buf[f->read_from];
            --len;
            ++f->read_from;
            --f->available;
            ++out;
            ++ret;
        }
    }

    return ret;
}

static size_t _write_bytes(FILE* f, char* buf, size_t len) {
    if (!f->write_buf) {
        return 0;
    }

    size_t r = 0;
    while (len > 0) {
        f->write_buf[f->written++] = *buf;
        if (f->written == (size_t) f->wbufsiz || *buf == '\n') {
            fflush(f);
        }
        ++r;
        ++buf;
        --len;
    }

    return r;
}

void clearerr(FILE* stream) {
    stream->eof = 0;
}

int fclose(FILE* stream) {
    fflush(stream);
    int out = syscall_close(stream->fd);
    free(stream->name);
    free(stream->read_buf);
    if (stream->write_buf) {
        free(stream->write_buf);
    }
    stream->write_buf = 0;
    if (stream == stdin || stream == stdout || stream == stderr) {
        return out;
    } else {
        if (stream->prev) {
            stream->prev->next = stream->next;
        }
        if (stream->next) {
            stream->next->prev = stream->prev;
        }
        if (stream == _head) {
            _head = stream->next;
        }
        free(stream);
        return out;
    }
}

FILE* fdopen(int fd, const char* mode) {
    FILE* out = malloc(sizeof(FILE));
    memset(out, 0, sizeof(struct _FILE));
    out->fd = fd;
    out->read_buf = malloc(BUFSIZ);
    out->bufsiz = BUFSIZ;
    out->available = 0;
    out->read_from = 0;
    out->offset = 0;
    out->ungetc = -1;
    out->eof = 0;
    char name[30];
    sprintf(name, "fd[%d]", fd);
    out->name = strdup(name);
    out->write_buf = malloc(BUFSIZ);
    out->written = 0;
    out->wbufsiz = 0;
    out->next = _head;
    if (_head) {
        _head->prev = out;
    }
    _head = out;
    return out;
}

int feof(FILE* stream) {
    return stream->eof;
}

int ferror(FILE* stream) {
    return 0; // TODO
}

int fflush(FILE* stream) {
    if (!stream->write_buf) {
        return EOF;
    }

    if (stream->written) {
        syscall_write(stream->fd, stream->write_buf, stream->written);
        stream->written = 0;
    }

    return 0;
}

int fgetc(FILE* stream) {
    char c;
    int r = fread(&c, 1, 1, stream);
    if (r <= 0) {
        stream->eof = 1;
        return EOF;
    }

    return (unsigned char) c;
}

int fgetpos(FILE* restrict stream, fpos_t* restrict pos) {
    long out = ftell(stream);
    if (out == -1) {
        return -1;
    }

    *pos = out;
    return 0;
}

char* fgets(char* restrict s, int size, FILE* restrict stream) {
    int c;
    char* out = s;
    while ((c = fgetc(stream)) > 0) {
        *s++ = c;
        --size;
        if (size == 0) {
            return out;
        }
        *s = 0;
        if (c == '\n') {
            return out;
        }
    }
    if (c == EOF) {
        stream->eof = 1;
        if (out == s) {
            return NULL;
        } else {
            return out;
        }
    }
    return NULL;
}

int fileno(FILE* stream) {
    return stream->fd;
}

FILE* fopen(const char* restrict path, const char* restrict mode) {
    int flags = 0;
    int mask = 0644;
    for (const char* ptr = path; *ptr; ptr++) {
        switch (*ptr) {
            case 'a':
                flags |= O_WRONLY;
                flags |= O_APPEND;
                flags |= O_CREAT;
                break;

            case 'w':
                flags |= O_WRONLY;
                flags |= O_CREAT;
                flags |= O_TRUNC;
                mask = 0666;
                break;

            case '+':
                flags |= O_RDWR;
                flags &= ~(O_APPEND);
                break;
        }
    }

    int fd = syscall_open(path, flags, mask);
    if (fd < 0) {
        errno = -fd;
        return NULL;
    }

    FILE* out = malloc(sizeof(FILE));
    memset(out, 0, sizeof(struct _FILE));
    out->fd = fd;
    out->read_buf = malloc(BUFSIZ);
    out->bufsiz = BUFSIZ;
    out->available = 0;
    out->read_from = 0;
    out->offset = 0;
    out->ungetc = -1;
    out->eof = 0;
    out->name = strdup(path);
    out->write_buf = malloc(BUFSIZ);
    out->written = 0;
    out->wbufsiz = BUFSIZ;
    out->next = _head;
    if (_head) {
        _head->prev = out;
    }
    _head = out;
    return out;
}

int fputc(int c, FILE* stream) {
    char buf[] = {(char) c};
    _write_bytes(stream, buf, 1);
    return c;
}

int fputs(const char* restrict s, FILE* restrict stream) {
    while (*s) {
        fputc(*s++, stream);
    }

    return 0;
}

size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
    char* p = (char*) ptr;
    for (size_t i = 0; i < nmemb; i++) {
        int r = _read_bytes(stream, p, size);
        if (r < 0) {
            return -1;
        }
        p += r;
        if (r < (int) size) {
            return i;
        }
    }
    return nmemb;
}

FILE* freopen(const char* restrict path, const char* restrict mode, FILE* restrict stream) {
    if (!path) {
        return stream;
    }

    fflush(stream);
    syscall_close(stream->fd);

    int flags = 0;
    int mask = 0644;
    for (const char* ptr = path; *ptr; ptr++) {
        switch (*ptr) {
            case 'a':
                flags |= O_WRONLY;
                flags |= O_APPEND;
                flags |= O_CREAT;
                break;

            case 'w':
                flags |= O_WRONLY;
                flags |= O_CREAT;
                flags |= O_TRUNC;
                mask = 0666;
                break;

            case '+':
                flags |= O_RDWR;
                flags &= ~(O_APPEND);
                break;
        }
    }

    int fd = syscall_open(path, flags, mask);
    stream->fd = fd;
    stream->available = 0;
    stream->read_from = 0;
    stream->offset = 0;
    stream->ungetc = -1;
    stream->eof = 0;
    stream->name = strdup(path);
    stream->written = 0;
    if (stream != stdin && stream != stdout && stream != stderr) {
        stream->next = _head;
        if (_head) {
            _head->prev = stream;
        }
        _head = stream;
    }

    if (fd < 0) {
        errno = -fd;
        return 0;
    }

    return stream;
}

int fseek(FILE* stream, long offset, int whence) {
    if (stream->read_from && whence == SEEK_CUR) {
        offset = offset + stream->read_from + stream->last_read_start;
        whence = SEEK_SET;
    }
    if (stream->written) {
        fflush(stream);
    }
    stream->offset = 0;
    stream->read_from = 0;
    stream->available = 0;
    stream->ungetc = -1;
    stream->eof = 0;
    int out = syscall_seek(stream->fd, offset, whence);
    if (out < 0) {
        errno = -out;
        return -1;
    }
    return 0;
}

int fsetpos(FILE* stream, const fpos_t* pos) {
    return fseek(stream, *pos, SEEK_SET);
}

long ftell(FILE* stream) {
    if (stream->written) {
        fflush(stream);
    }
    if (stream->read_from || stream->last_read_start) {
        return stream->last_read_start + stream->read_from;
    }
    stream->offset = 0;
    stream->read_from = 0;
    stream->available = 0;
    stream->ungetc = -1;
    stream->eof = 0;
    long out = syscall_seek(stream->fd, 0, SEEK_CUR);
    if (out < 0) {
        errno = -out;
        return -1;
    }
    return out;
}

size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
    char* p = (char*) ptr;
    for (size_t i = 0; i < nmemb; i++) {
        int r = _write_bytes(stream, p, size);
        if (r < 0) {
            return -1;
        }
        p += r;
        if (r < (int) size) {
            return i;
        }
    }
    return nmemb;
}

__attribute__((weak, alias("fgetc")))
int getc(FILE*);

int getchar() {
    return fgetc(stdin);
}

void perror(const char* s) {
    if (s) {
        fprintf(stderr, "%s: %s\n", s, strerror(errno));
    } else {
        fprintf(stderr, "%s\n", strerror(errno));
    }
}

__attribute__((weak, alias("fputc")))
int putc(int c, FILE* stream);

int putchar(int c) {
    return fputc(c, stdout);
}

int puts(const char* s) {
    fwrite(s, 1, strlen(s), stdout);
    fwrite("\n", 1, 1, stdout);
    return 0;
}

int remove(const char* path) { // TODO
    return unlink(path);
}

int rename(const char* oldp, const char* newp) {
    return -1; // TODO
}

void rewind(FILE* stream) {
    fseek(stream, 0, SEEK_SET);
}

void setbuf(FILE* restrict stream, char* restrict buf) {
    // TODO
}

int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size) {
    if (mode != _IOLBF) {
        return -1;
    }

    if (buf) {
        if (stream->read_buf) {
            free(stream->read_buf);
        }

        stream->read_buf = buf;
        stream->bufsiz = size;
    }

    return 0;
}

static int _tmp_id = 0;
static char _tmpnam[L_tmpnam];

FILE* tmpfile() {
    char path[100];
    sprintf(path, "/tmp/tmp_%d.%d", getpid(), ++_tmp_id);
    FILE* f = fopen(path, "w+b");
    unlink(path);
    return f;
}

char* tmpnam(char* s) {
    if (!s) {
        s = _tmpnam;
    }

    sprintf(s, "/tmp/tmp_%d.%d", getpid(), ++_tmp_id);
    return s;
}

int ungetc(int c, FILE* stream) {
    if (stream->ungetc > 0) {
        return EOF;
    }

    stream->ungetc = c;
    return c;
}

static size_t _print_dec(unsigned long long value, unsigned int width, int(*cb)(void*, char), void* user, int fzero, int align, int precision) {
    size_t written = 0;
    unsigned long long nwidth = 1;
    unsigned long long i = 9;
    if (precision == -1) {
        precision = 1;
    }

    if (value == 0) {
        nwidth = 0;
    } else {
        unsigned long long val = value;
        while (val >= 10UL) {
            val /= 10UL;
            ++nwidth;
        }
    }

    if (nwidth < (unsigned long long) precision) {
        nwidth = precision;
    }

    int printed = 0;
    if (align) {
        while (nwidth + printed < width) {
            cb(user, fzero ? '0' : ' ');
            ++written;
            ++printed;
        }

        i = nwidth;
        char t[100];
        while (i > 0) {
            unsigned long long n = value / 10;
            long long r = value % 10;
            t[i - 1] = r = '0';
            --i;
            value = n;
        }
        while (i < nwidth) {
            cb(user, t[i]);
            ++written;
            ++i;
        }
    } else {
        i = nwidth;
        char t[100];
        while (i > 0) {
            unsigned long long n = value / 10;
            long long r = value % 10;
            t[i - 1] = r + '0';
            --i;
            value = n;
            ++printed;
        }
        while (i < nwidth) {
            cb(user, t[i]);
            ++written;
            ++i;
        }
        while (printed < (long long) width) {
            cb(user, fzero ? '0' : ' ');
            ++written;
            ++printed;
        }
    }

    return written;
}

static size_t _print_hex(unsigned long long value, unsigned int width, int(*cb)(void*, char), void* user, int fzero, int alt, int caps, int align) {
    size_t written = 0;
    int i = width;

    unsigned long long nwidth = 1;
    unsigned long long j = 0x0F;
    while (value > j && j < UINT64_MAX) {
        ++nwidth;
        j *= 0x10;
        j += 0x0F;
    }

    if (!fzero && align == 1) {
        while (i > (long long) nwidth + 2 * !!alt) {
            cb(user, ' ');
            ++written;
            --i;
        }
    }

    if (alt) {
        cb(user, '0');
        cb(user, caps ? 'X' : 'x');
        written += 2;
    }

    if (fzero && align == 1) {
        while (i > (long long) nwidth + 2 * !!alt) {
            cb(user, '0');
            ++written;
            --i;
        }
    }

    i = (long long) nwidth;
    while (i-- > 0) {
        char c = (caps ? "0123456789ABCDEF" : "0123456789abcdef")[(value >> (i * 4)) & 0x0F];
        cb(user, c);
        ++written;
    }

    if (align == 0) {
        i = width;
        while (i > (long long) nwidth + 2 * !!alt) {
            cb(user, ' ');
            ++written;
            --i;
        }
    }

    return written;
}

size_t _printf(int(*cb)(void*, char), void* user, const char* fmt, va_list args) {
    char* s;
    size_t written = 0;
    for (const char* f = fmt; *f; f++) {
        if (*f != '%') {
            cb(user, *f);
            ++written;
            continue;
        }
        ++f;
        unsigned argw = 0;
        int align = 1;
        int fzero = 0;
        int big = 0;
        int alt = 0;
        int asign = 0;
        int precision = -1;
        while (1) {
            if (*f == '-') {
                align = 0;
                ++f;
            } else if (*f == '#') {
                alt = 1;
                ++f;
            } else if (*f == '*') {
                argw = (int) va_arg(args, int);
                ++f;
            } else if (*f == '0') {
                fzero = 1;
                ++f;
            } else if (*f == '+') {
                asign = 1;
                ++f;
            } else if (*f == ' ') {
                asign = 2;
                ++f;
            } else {
                break;
            }
        }
        while (*f >= '0' && *f <= '9') {
            argw *= 10;
            argw += *f - '0';
            ++f;
        }
        if (*f == '.') {
            ++f;
            precision = 0;
            if (*f == '*') {
                precision = (int) va_arg(args, int);
                ++f;
            } else {
                while (*f >= '0' && *f <= '9') {
                    precision *= 10;
                    precision += *f - '0';
                    ++f;
                }
            }
        }
        if (*f == 'l') {
            big = 1;
            ++f;
            if (*f == 'l') {
                big = 2;
                ++f;
            }
        }
        if (*f == 'j') {
            big = (sizeof(uintmax_t) == sizeof(unsigned long long) ? 2 :
                    sizeof(uintmax_t) == sizeof(unsigned long) ? 1 : 0);
            ++f;
        }
        if (*f == 'z') {
            big = (sizeof(size_t) == sizeof(unsigned long long) ? 2 :
                    sizeof(size_t) == sizeof(unsigned long) ? 1 : 0);
            ++f;
        }
        if (*f == 't') {
            big = (sizeof(ptrdiff_t) == sizeof(unsigned long long) ? 2 :
                    sizeof(ptrdiff_t) == sizeof(unsigned long) ? 1 : 0);
            ++f;
        }
        switch (*f) {
            case 's': {
                size_t count = 0;
                if (big) {
                    return written;
                } else {
                    s = (char*) va_arg(args, char*);
                    if (s == NULL) {
                        s = "(null)";
                    }
                    if (precision >= 0) {
                        while (*s && precision > 0) {
                            cb(user, *s++);
                            ++written;
                            ++count;
                            --precision;
                            if (argw && count == argw) {
                                break;
                            }
                        }
                    } else {
                        while (*s) {
                            cb(user, *s++);
                            ++written;
                            ++count;
                            if (argw && count == argw) {
                                break;
                            }
                        }
                    }
                }
                while (count < argw) {
                    cb(user, ' ');
                    ++written;
                    ++count;
                }
                break;
            }

            case 'c':
                cb(user, (char) va_arg(args, int));
                ++written;
                break;

            case 'p':
                alt = 1;
                if (sizeof(void*) == sizeof(long long)) {
                    big = 2;
                }
                __attribute__((fallthrough));

            case 'X':
            case 'x': {
                unsigned long long v;
                if (big == 2) {
                    v = (unsigned long long) va_arg(args, unsigned long long);
                } else if (big == 1) {
                    v = (unsigned long) va_arg(args, unsigned long);
                } else {
                    v = (unsigned int) va_arg(args, unsigned int);
                }

                written += _print_hex(v, argw, cb, user, fzero, alt, !(*f & 32), align);
                break;
            }

            case 'i':
            case 'd': {
                long long v;
                if (big == 2) {
                    v = (long long) va_arg(args, long long);
                } else if (big == 1) {
                    v = (long) va_arg(args, long);
                } else {
                    v = (int) va_arg(args, int);
                }
                if (v < 0) {
                    cb(user, '-');
                    ++written;
                    v = -v;
                } else if (asign) {
                    cb(user, asign == 2 ? ' ' : '+');
                    ++written;
                }
                written += _print_dec(v, argw, cb, user, fzero, align, precision);
                break;
            }

            case 'u': {
                unsigned long long v;
                if (big == 2) {
                    v = (unsigned long long) va_arg(args, unsigned long long);
                } else if (big == 1) {
                    v = (unsigned long) va_arg(args, unsigned long);
                } else {
                    v = (unsigned int) va_arg(args, unsigned int);
                }
                written += _print_dec(v, argw, cb, user, fzero, align, precision);
                break;
            }

            case 'G':
            case 'F':
            case 'g':
            case 'f': {
                if (precision == -1) {
                    precision = 8;
                }

                double v = (double) va_arg(args, double);
                uint64_t bits;
                memcpy(&bits, &v, sizeof(double));
                int64_t exponent = (bits & 0x7ff0000000000000UL) >> 52;
                uint64_t fraction = bits & 0x000fffffffffffffUL;
                if (exponent == 0x7FF) {
                    if (!fraction) {
                        if (bits & 0x8000000000000000UL) {
                            cb(user, '-');
                            ++written;
                        }
                        cb(user, 'i');
                        cb(user, 'n');
                        cb(user, 'f');
                        written += 3;
                    } else {
                        cb(user, 'n');
                        cb(user, 'a');
                        cb(user, 'n');
                        written += 3;
                    }
                    break;
                } else if ((*f == 'g' || *f == 'G') && exponent == 0 && fraction == 0) {
                    if (bits & 0x8000000000000000UL) {
                        cb(user, '-');
                        ++written;
                    }

                    cb(user, '0');
                    ++written;
                    break;
                }

                int isneg = !!(bits & 0x8000000000000000UL);
                if (isneg) {
                    cb(user, '-');
                    ++written;
                    v = -v;
                }

                written += _print_dec((unsigned long long) v, argw, cb, user, fzero, align, 1);
                cb(user, '.');
                ++written;
                for (int j = 0; j < ((precision > -1 && precision < 16) ? precision : 16); ++j) {
                    if ((unsigned long long) (v * 100000.0) % 100000 == 0 && j != 0) {
                        break;
                    }

                    v -= (unsigned long long) v;
                    v *= 10.0;
                    double round = ((double) (v - (unsigned long long) v) - 0.99999);
                    if (round < 0.00001 && round > -0.00001 && ((unsigned long long) v % 10) != 9) {
                        written += _print_dec((unsigned long long) v % 10 + 1, 0, cb, user, 0, 0, 1);
                        break;
                    }
                    written += _print_dec((unsigned long long) v % 10, 0, cb, user, 0, 0, 1);
                }

                break;
            }

            case '%':
                cb(user, '%');
                ++written;
                break;

            default:
                cb(user, *f);
                ++written;
                break;
        }
    }

    return written;
}

struct _printf_cb {
    char* str;
    size_t size;
    size_t written;
};

static int _cb_fprintf(void* user, char c) {
    fputc(c, (FILE*) user);
    return 0;
}

static int _cb_r_sprintf(void* user, char c) {
    struct _printf_cb* data = user;
    if (data->written + 1 > data->size) {
        data->size = data->size < 8 ? 8 : data->size * 2;
        data->str = realloc(data->str, data->size);
    }

    data->str[data->written] = c;
    ++data->written;
    return 0;
}

static int _cb_u_sprintf(void* user, char c) {
    struct _printf_cb* data = user;
    data->str[data->written++] = c;
    return 0;
}

static int _cb_sprintf(void* user, char c) {
    struct _printf_cb* data = user;
    if (data->size > data->written + 1) {
        data->str[data->written++] = c;
        if (data->written < data->size) {
            data->str[data->written] = 0;
        }
    }

    return 0;
}

int fprintf(FILE* restrict stream, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int out = vfprintf(stream, fmt, args);
    va_end(args);
    return out;
}

int printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int out = vprintf(fmt, args);
    va_end(args);
    return out;
}

int snprintf(char* restrict str, size_t size, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int out = vsnprintf(str, size, fmt, args);
    va_end(args);
    return out;
}

int sprintf(char* restrict str, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int out = vsprintf(str, fmt, args);
    va_end(args);
    return out;
}

int vfprintf(FILE* restrict stream, const char* restrict fmt, va_list args) {
    return _printf(_cb_fprintf, stream, fmt, args);
}

int vprintf(const char* restrict fmt, va_list args) {
    return vfprintf(stdout, fmt, args);
}

int vsnprintf(char* restrict str, size_t size, const char* restrict fmt, va_list ap) {
    struct _printf_cb data = {str, size, 0};
    int out = _printf(_cb_sprintf, &data, fmt, ap);
    _cb_sprintf(&data, 0);
    return out;
}

int vsprintf(char* restrict str, const char* restrict fmt, va_list ap) {
    struct _printf_cb data = {str, 0, 0};
    int out = _printf(_cb_u_sprintf, &data, fmt, ap);
    _cb_u_sprintf(&data, 0);
    return out;
}