/*
 * stdlib.h - standard library definitions
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_wait.h.html
 */

#ifndef _STDLIB_H
#define _STDLIB_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/wait.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RAND_MAX 2147483647

#define MB_CUR_MAX 1

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

void _Exit(int code);
// long a64l(const char*);
void abort();
int abs(int x);
int atexit(void(*func)(void));
double atof(const char*); // TODO: Implementation
int atoi(const char* s);
long atol(const char*); // TODO: Implementation
// long long atoll(const char*);
void* bsearch(const void*, const void*, size_t, size_t, int(*)(const void*, const void*)); // TODO: Implementation
void* calloc(size_t size, size_t nmemb);
div_t div(int, int); // TODO: Implementation
// double drand48();
// double erand48(unsigned short[3]);
void exit(int code);
void free(void* ptr);
char* getenv(const char* s);
// int getsubopt(char**, char* const*, char**);
// int grantpt(int);
// char* initstate(unsigned, char*, size_t);
// long jrand48(unsigned short[3]);
// char* l64a(long);
long labs(long); // TODO: Implementation
// void lcong48(unsigned short[7]);
ldiv_t ldiv(long, long); // TODO: Implementation
// long long llabs(long long);
// lldiv_t lldiv(long long, long long);
// long lrand48();
void* malloc(size_t size);
// int mblen(const char*, size_t);
// size_t mbstowcs(wchar_t* restrict, const char* restrict, size_t);
// int mbtowc(wchar_t* restrict, const char* restrict, size_t);
// char* mkdtemp(char*);
// int mkstemp(char*);
// long mrand48();
// long nrand48(unsigned short[3]);
// int posix_memalign(void**, size_t, size_t);
// int posix_openpt(int);
// char* ptsname(int);
// int putenv(char*);
void qsort(void*, size_t, size_t, int(*)(const void*, const void*)); // TODO: Implementation
int rand(); // TODO: Implementation
// int rand_r(unsigned*);
// long random();
void* realloc(void* ptr, size_t size);
// char* realpath(const char* restrict, char* restrict);
// unsigned short* seed48(unsigned short[3]);
// int setenv(const char*, const char*, int);
// void setkey(const char*);
// char* setstate(char*);
void srand(unsigned); // TODO: Implementation
// void srand48(long);
// void srandom(unsigned);
double strtod(const char* /*restrict*/, char** /*restrict*/); // TODO: Implementation
// float strtof(const char* restrict, char** restrict);
long strtol(const char* /*restrict*/, char** /*restrict*/, int); // TODO: Implementation
// long double strtold(const char* restrict, char** restrict);
// long long strtoll(const char* restrict, char** restrict, int);
unsigned long strtoul(const char* /*restrict*/, char** /*restrict*/, int); // TODO: Implementation
// unsigned long long strtoull(const char* restrict, char** restrict, int);
int system(const char*); // TODO: Implementation
// int unlockpt(int);
// int unsetenv(const char*);
// size_t wcstombs(char* restrict, const wchar_t* restrict, size_t);
// int wctomb(char*, wchar_t);

#ifdef __cplusplus
}
#endif

#endif