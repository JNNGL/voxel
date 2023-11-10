#include <sys/utsname.h>
#include <sys/syscall.h>

int uname(struct utsname* name) {
    return syscall_uname(name);
}