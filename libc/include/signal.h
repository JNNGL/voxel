#ifndef _SIGNAL_H
#define _SIGNAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <time.h>

typedef int sig_atomic_t;
typedef unsigned sigset_t;

union sigval {
    int sival_int;
    void* sival_ptr;
};

typedef struct {
    int si_signo;
    int si_code;
    int si_errno;
    pid_t si_pid;
    uid_t si_uid;
    void* si_addr;
    int si_status;
    long si_band;
    union sigval si_value;
} siginfo_t;

struct sigevent {
    int sigev_notify;
    int sigev_signo;
    union sigval sigev_value;
    void(*sigev_notify_function)(union sigval);
//    pthread_attr_t* sigev_notify_attributes;
};

struct sigaction {
    void(*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void(*sa_sigaction)(int, siginfo_t*, void*);
};

#define SIG_DFL (((void)(*)(int)) 0)
#define SIG_IGN (((void)(*)(int)) 1)
#define SIG_ERR (((void)(*)(int)) -1)

#define SIGEV_NONE   0
#define SIGEV_SIGNAL 1
#define SIGEV_THREAD 3

#define SI_USER    1
#define SI_QUEUE   2
#define SI_TIMER   3
#define SI_ASYNCIO 4
#define SI_MESGQ   5

#define SA_NOCLDSTOP 1
#define SA_SIGINFO   2
#define SA_NODEFER   3
#define SA_RESETHAND 8
#define SA_RESTART   9

#define SIG_SETMASK 0
#define SI_BLOCK    1
#define SIG_UNBLOCK 2

#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGEMT      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGBUS      10
#define SIGSEGV     11
#define SIGSYS      12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGUSR1     16
#define SIGUSR2     17
#define SIGCHLD     18
#define SIGPWR      19
#define SIGWINCH    20
#define SIGURG      21
#define SIGPOLL     22
#define SIGSTOP     23
#define SIGTSTP     24
#define SIGCONT     25
#define SIGTTIN     26
#define SIGTTOUT    27
#define SIGVTALRM   28
#define SIGPROF     29
#define SIGXCPU     30
#define SIGXFSZ     31
#define SIGWAITING  32
#define SIGDIAF     33
#define SIGHATE     34
#define SIGWINEVENT 35
#define SIGCAT      36
#define SIGTTOU     37

#define NSIG 38

// TODO: Implementation
// Prototypes just for libstdc++ building.
int kill(pid_t, int);
int killpg(pid_t, int);
void psiginfo(const siginfo_t*, const char*);
void psignal(int, const char*);
//int pthread_kill(pthread_t*, int);
//int pthread_sigmask(int, const sigset_t* /*restrict*/, sigset_t* /*restrict*/);
int raise(int);
int sigaction(int, const struct sigaction* /*restrict*/, struct sigaction* /*restrict*/);
int sigaddset(sigset_t*, int);
int sigdelset(sigset_t*, int);
int sigemptyset(sigset_t*);
int sigfillset(sigset_t*);
int sighold(int);
int sigignore(int);
int siginterrupt(int, int);
int sigismember(const sigset_t*, int);
void(*signal(int, void(*)(int)))(int);
int sigpause(int);
int sigpending(sigset_t*);
int sigprocmask(int, const sigset_t* /*restrict*/, sigset_t* /*restrict*/);
int sigqueue(pid_t, int, union sigval);
int sigrelse(int);
void(*sigset(int, void(*)(int)))(int);
int sigsuspend(const sigset_t*);
int sigtimedwait(const sigset_t* /*restrict*/, siginfo_t* /*restrict*/, const struct timespec* /*restrict*/);
int sigwait(const sigset_t* /*restrict*/, int* /*restrict*/);
int sigwaitinfo(const sigset_t* /*restrict*/, siginfo_t* /*restrict*/);

#ifdef __cplusplus
}
#endif

#endif