/*
 * sys/wait.h - declarations for waiting
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_wait.h.html
 */

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H 1

// TODO: #include <signal.h>
#include <sys/types.h>

#define WNOHANG    0x0001
#define WUNTRACED  0x0002
#define WCONTINUED 0x0010

#define WEXITSTATUS(w) (((w) >> 8) & 0xFF)
#define WIFEXITED(w)   (((w) & 0xFF) == 0)
#define WIFSIGNALED(w) (((w) & 0x7F) > 0 && (((w) & 0x7F) < 0x7F))
#define WIFSTOPPED(w)  (((w) & 0xFF) == 0x7F)
#define WTERMSIG(w)    ((w) & 0x7F)

#define WEXITED  0x0004
#define WSTOPPED 0x0008
#define WNOWAIT  0x0020

// pid_t wait(int*);
// int waitid(idtype_t, id_t, siginfo_t*, int);
// pid_t waitpid(pid_t, int*, int);

#endif