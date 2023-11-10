#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct ringbuffer {
    uint8_t* buffer;
    size_t ridx;
    size_t widx;
    size_t size;
} ringbuffer_t;

ringbuffer_t* ringbuffer_alloc(size_t s);
size_t ringbuffer_avail(ringbuffer_t* buf);
size_t ringbuffer_readable(ringbuffer_t* buf);
size_t ringbuffer_write(ringbuffer_t* buf, size_t size, uint8_t* buffer);
size_t ringbuffer_read(ringbuffer_t* buf, size_t size, uint8_t* buffer);
void ringbuffer_free(ringbuffer_t* buf);
