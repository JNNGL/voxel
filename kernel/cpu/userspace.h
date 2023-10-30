#pragma once

#include <stdint.h>
#include <stddef.h>

void enter_userspace(uintptr_t entrypoint, int argc, char* argv[], char* envp[], uintptr_t stack);