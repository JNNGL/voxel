#include <stdint.h>

extern void _init();
extern void _fini();

static const char* __env_empty[4] = {0};

char** environ = 0;
int _environ_size = 0;
char* __argv0 = 0;

char** __argv = 0;
char** __get_argv() {
    return __argv;
}

void _exit(int rval) {
    _fini();
    asm volatile("int $0x80" : : "a"(0), "b"(rval));
    __builtin_unreachable();
}

void _libc_init() {
    uint32_t zeros = 0;
    for (uint32_t x = 0; 1; ++x) {
        if (!__get_argv()[x]) {
            if (++zeros == 2) {
                break;
            }

            continue;
        }

        if (zeros == 1) {
            environ = &__get_argv()[x];
            break;
        }
    }

    if (environ) {
        int size = 0;
        char** envp = environ;
        while (*envp++ && ++size);
        // TODO: Add extra entries
    } else {
        environ = (char**) __env_empty;
    }

    __argv0 = __get_argv()[0];
}

void _main(int argc, char* argv[], char** envp, int(*main)(int, char**)) {
    if (!__get_argv()) {
        __argv = argv;
        _libc_init(); // TODO: Fix global constructors
    }

    _init();
    int rval = main(argc, argv);
    _exit(rval);
}