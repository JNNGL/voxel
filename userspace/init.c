#include <sys/syscall.h>
#include <unistd.h>

void init_stdio_fds() {
    // TODO: Don't open /dev/pts/1 explicitly
    syscall_open("/dev/pts/1", 0, 0);
    syscall_open("/dev/pts/1", 1, 0);
    syscall_open("/dev/pts/1", 1, 0);
}

int main(int argc, const char* argv[]) {
    init_stdio_fds();

    pid_t child = fork();
    if (child == 0) {
        char* argv[] = {"/bin/sh", 0};
        execv("/bin/sh", argv);
        return 1;
    }

    while (1);

    return 0;
}