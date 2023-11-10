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
    extern void __stdio_free();
    __stdio_free();

    _fini();
    asm volatile("int $0x80" : : "a"(0), "b"(rval));
    __builtin_unreachable();
}

__attribute__((constructor))
void _libc_init() {
    extern void __stdio_init();
    __stdio_init();

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

        extern uintptr_t __init_array_start;
        extern uintptr_t __init_array_end;
        for (uintptr_t* ctor = &__init_array_start; ctor < &__init_array_end; ++ctor) {
            ((void(*)(void)) *ctor)();
        }
    }

    _init();
    int rval = main(argc, argv);
    _exit(rval);
}