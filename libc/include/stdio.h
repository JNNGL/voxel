/*
 * stdio.h - standard buffered input/output
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
 */

#ifndef _STDIO_H
#define _STDIO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <stddef.h>

typedef struct _FILE FILE;
typedef long fpos_t;

#define BUFSIZ 8192

#define L_ctermid 256
#define L_tmpnam  256

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FILENAME_MAX 1024

#define EOF (-1)

#define P_tmpdir "/tmp/"

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

void clearerr(FILE* stream);
// char* ctermid(char*);
// int dprintf(int, const char* restrict, ...);
int fclose(FILE* stream);
FILE* fdopen(int fd, const char* mode);
int feof(FILE* stream);
int ferror(FILE* stream);
int fflush(FILE* stream);
int fgetc(FILE* stream);
int fgetpos(FILE* /*restrict*/ stream, fpos_t* /*restrict*/ pos);
char* fgets(char* /*restrict*/ s, int size, FILE* /*restrict*/ stream);
int fileno(FILE* stream);
// void flockfile(FILE*);
// FILE* fmemopen(void* restrict, size_t, const char* restrict);
FILE* fopen(const char* /*restrict*/ path, const char* /*restrict*/ mode);
int fprintf(FILE* /*restrict*/ stream, const char* /*restrict*/ fmt, ...);
int fputc(int c, FILE* stream);
int fputs(const char* /*restrict*/ s, FILE* /*restrict*/ stream);
size_t fread(void* /*restrict*/ ptr, size_t size, size_t nmemb, FILE* /*restrict*/ stream);
FILE* freopen(const char* /*restrict*/ path, const char* /*restrict*/ mode, FILE* /*restrict*/ stream);
int fscanf(FILE* /*restrict*/, const char* /*restrict*/, ...); // TODO: Implementation
int fseek(FILE* stream, long offset, int whence);
// int fseeko(FILE*, off_t, int);
int fsetpos(FILE* stream, const fpos_t* pos);
long ftell(FILE* stream);
// off_t ftello(FILE*);
// int ftrylockfile(FILE*);
// int funlockfile(FILE*);
size_t fwrite(const void* /*restrict*/ ptr, size_t size, size_t nmemb, FILE* /*restrict*/ stream);
int getc(FILE* stream);
int getchar();
// int getc_unlocked(FILE*);
// int getchar_unlocked();
// ssize_t getdelim(char** restrict, size_t* restrict, int, FILE* restrict);
// ssize_t getline(char** restrict, size_t* restrict, FILE* restrict);
// char* gets(char*);
// FILE* open_memstream(char**, size_t*);
// int pclose(FILE*);
void perror(const char* s);
// FILE* popen(const char*, const char*);
int printf(const char* /*restrict*/ fmt, ...);
int putc(int c, FILE* stream);
int putchar(int c);
// int putc_unlocked(int, FILE*);
// int putchar_unlocked(int);
int puts(const char* s);
int remove(const char* path);
int rename(const char* oldp, const char* newp);
// int renameat(int, const char*, int, const char*);
void rewind(FILE* stream);
int scanf(const char* /*restrict*/, ...); // TODO: Implementation
void setbuf(FILE* /*restrict*/ stream, char* /*restrict*/ buf);
int setvbuf(FILE* /*restrict*/ stream, char* /*restrict*/ buf, int mode, size_t size);
int snprintf(char* /*restrict*/ str, size_t size, const char* /*restrict*/ fmt, ...);
int sprintf(char* /*restrict*/ str, const char* /*restrict*/ fmt, ...);
int sscanf(const char* /*restrict*/, const char* /*restrict*/, ...); // TODO: Implementation
// char* tempnam(const char*, const char*);
FILE* tmpfile();
char* tmpnam(char* s);
int ungetc(int c, FILE* stream);
// int vdprintf(int, const char* restrict, va_list);
int vfprintf(FILE* /*restrict*/ stream, const char* /*restrict*/ fmt, va_list args);
// int vfscanf(FILE* restrict, const char* restrict, va_list);
int vprintf(const char* /*restrict*/ fmt, va_list args);
// int vscanf(const char* restrict, va_list);
int vsnprintf(char* /*restrict*/ str, size_t size, const char* /*restrict*/ fmt, va_list ap);
int vsprintf(char* /*restrict*/ str, const char* /*restrict*/ fmt, va_list ap);
// int vsscanf(const char* restrict, const char* restrict, va_list);

#ifdef __cplusplus
}
#endif

#endif