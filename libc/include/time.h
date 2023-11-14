#ifndef _TIME_H
#define _TIME_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <locale.h>

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct timespec {
    time_t tv_sev;
    long tv_nsec;
};

struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};

typedef int clockid_t;

#define CLOCKS_PER_SEC 1000000

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

// TODO: Implementation
// Prototypes just for libstdc++ building.
extern int daylight;
extern long timezone;
extern char* tzname[];

char* asctime(const struct tm*);
char* asctime_r(const struct tm* /*restrict*/, char* /*restrict*/);
clock_t clock();
int clock_getcpuclockid(pid_t, clockid_t*);
int clock_getres(clockid_t, struct timespec*);
int clock_gettime(clockid_t, struct timespec*);
int clock_nanosleep(clockid_t, int, const struct timespec*, struct timespec*);
int clock_settime(clockid_t, const struct timespec*);
char* ctime(const time_t*);
char* ctime_r(const time_t*, char*);
double difftime(time_t, time_t);
struct tm* getdate(const char*);
struct tm* gmtime(const time_t);
struct tm* gmtime_r(const time_t* /*restrict*/, struct tm* /*restrict*/);
struct tm* localtime(const time_t*);
struct tm* localtime_r(const time_t* /*restrict*/, struct tm* /*restrict*/);
time_t mktime(struct tm*);
int nanosleep(const struct timespec*, struct timespec*);
size_t strftime(char* /*restrict*/, size_t, const char* /*restrict*/, const struct tm* /*restrict*/);
//size_t strftime_l(char* restrict, size_t, const char* restrict, const struct tm* restrict, locale_t);
char* strptime(const char* /*restrict*/, const char* /*restrict*/, struct tm* /*restrict*/);
time_t time(time_t*);
//int timer_create(clockid_t, struct sigevent* restrict, timer_t* restrict);
int timer_delete(timer_t);
int timer_getoverrun(timer_t);
int timer_gettime(timer_t, struct itimerspec*);
int timer_settime(timer_t, int, const struct itimerspec* /*restrict*/, struct itimerspec* /*restrict*/);

#ifdef __cplusplus
}
#endif

#endif