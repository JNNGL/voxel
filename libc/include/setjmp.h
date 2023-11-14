#ifndef _SETJMP_H
#define _SETJMP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define _JBLEN 9

typedef long long jmp_buf[_JBLEN];
typedef long long sigjmp_buf[_JBLEN];

// TODO: Implementation
// Prototypes just for libstdc++ building.
void _longjmp(jmp_buf, int);
void longjmp(jmp_buf, int);
void siglongjmp(sigjmp_buf, int);

int _setjmp(jmp_buf);
int setjmp(jmp_buf);
int sigsetjmp(sigjmp_buf, int);

#ifdef __cplusplus
}
#endif

#endif