#include "userspace.h"

#include <cpu/idt.h>

void enter_userspace(uintptr_t entrypoint, int argc, char* argv[], char* envp[], uintptr_t stack) {
    struct regs ret;
    ret.cs = 0x18 | 0x03;
    ret.ss = 0x20 | 0x03;
    ret.rip = entrypoint;
    ret.rflags = (1 << 21) | (1 << 9);
    ret.rsp = stack;

    asm volatile("pushq %0\n"
                 "pushq %1\n"
                 "pushq %2\n"
                 "pushq %3\n"
                 "pushq %4\n"
                 "swapgs\n"
                 "iretq\n"
                 : : "m"(ret.ss),
                     "m"(ret.rsp),
                     "m"(ret.rflags),
                     "m"(ret.cs),
                     "m"(ret.rip),
                     "D"(argc), "S"(argv),
                     "d"(envp));
}