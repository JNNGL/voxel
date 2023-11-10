#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>

#define SWITCH_KERN_NAME    0x01
#define SWITCH_NODENAME     0x02
#define SWITCH_KERN_RELEASE 0x04
#define SWITCH_KERN_VER     0x08
#define SWITCH_MACHINE      0x10

void usage() {
    puts("usage: uname [-asnrvmo]");
    exit(0);
}

int decode_switches(int argc, const char** args) {
    int result = 0;

    for (int i = 0; i < argc; i++) {
        const char* arg = args[i];
        if (*arg != '-') {
            usage();
        }

        ++arg;
        for (const char* p = arg; *p; p++) {
            switch (*p) {
                case 'a':
                    return SWITCH_KERN_NAME
                           | SWITCH_NODENAME
                           | SWITCH_KERN_RELEASE
                           | SWITCH_KERN_VER
                           | SWITCH_MACHINE;

                case 'o':
                case 's':
                    result |= SWITCH_KERN_NAME;
                    break;

                case 'n':
                    result |= SWITCH_NODENAME;
                    break;

                case 'r':
                    result |= SWITCH_KERN_RELEASE;
                    break;

                case 'v':
                    result |= SWITCH_KERN_VER;
                    break;

                case 'm':
                    result |= SWITCH_MACHINE;
                    break;

                default:
                    usage();
            }
        }
    }

    if (!result) {
        result |= SWITCH_KERN_NAME;
    }

    return result;
}

int main(int argc, const char** argv) {
    int switches = decode_switches(argc - 1, &argv[1]);

    struct utsname utsname;
    uname(&utsname);

    if (switches & SWITCH_KERN_NAME) {
        printf("%s ", utsname.sysname);
    }

    if (switches & SWITCH_NODENAME) {
        printf("%s ", utsname.nodename);
    }

    if (switches & SWITCH_KERN_VER) {
        printf("%s ", utsname.version);
    }

    if (switches & SWITCH_KERN_RELEASE) {
        printf("%s ", utsname.release);
    }

    if (switches & SWITCH_MACHINE) {
        printf("%s ", utsname.machine);
    }

    printf("\n");
    return 0;
}