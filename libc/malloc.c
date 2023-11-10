#include <stdlib.h>

#include <sys/syscall.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// TODO: Actually write an allocator

__attribute__((malloc))
void* malloc(size_t size) {
    size_t unaligned_size = size;
    size += sizeof(size_t);
    if (size % 0x1000) {
        size -= size % 0x1000;
        size += 0x1000;
    }

    size_t* ptr = (size_t*)(uintptr_t) syscall_sbrk(size);
    *ptr++ = unaligned_size;
    return ptr;
}

__attribute__((malloc))
void* calloc(size_t size, size_t nmemb) {
    void* ptr = malloc(size * nmemb);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0, size * nmemb);
    return ptr;
}

__attribute__((malloc))
void* realloc(void* ptr, size_t size) {
    if (!ptr) {
        return NULL;
    }

    if (!size) {
        free(ptr);
        return NULL;
    }

    size_t oldsize = *((size_t*) ptr - 1);
    void* newptr = malloc(size);
    memcpy(newptr, ptr, oldsize < size ? oldsize : size);
    free(ptr);
    return newptr;
}

void free(void* ptr) {
    // Stub.
}