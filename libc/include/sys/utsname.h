/*
 * sys/utsname.h - system name structure
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_utsname.h.html
 */

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H 1

#define _SYS_NAMELEN 256

struct utsname {
    char sysname[_SYS_NAMELEN];
    char nodename[_SYS_NAMELEN];
    char release[_SYS_NAMELEN];
    char version[_SYS_NAMELEN];
    char machine[_SYS_NAMELEN];
};

int uname(struct utsname* name);

#endif