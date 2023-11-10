#include "ringbuffer.h"

#include <lib/alloc.h>
#include <lib/string.h>

ringbuffer_t* ringbuffer_alloc(size_t s) {
    ringbuffer_t* rb = malloc(s + sizeof(ringbuffer_t));
    rb->buffer = (uint8_t*) (rb + 1);
    rb->size = s;
    rb->ridx = 0;
    rb->widx = 0;
    return rb;
}

size_t ringbuffer_avail(ringbuffer_t* buf) {
    if (buf->ridx == buf->widx) {
        return buf->size - 1;
    }

    if (buf->ridx > buf->widx) {
        return buf->ridx - buf->widx - 1;
    } else {
        return (buf->size - buf->widx) + buf->ridx - 1;
    }
}

size_t ringbuffer_readable(ringbuffer_t* buf) {
    if (buf->ridx == buf->widx) {
        return 0;
    }

    if (buf->ridx > buf->widx) {
        return (buf->size - buf->ridx) + buf->widx;
    } else {
        return buf->widx - buf->ridx;
    }
}

size_t ringbuffer_write(ringbuffer_t* buf, size_t size, uint8_t* buffer) {
    size_t avail = ringbuffer_avail(buf);
    if (avail < size) {
        size = avail;
    }

    size_t to_write = size;
    if (buf->widx + size >= buf->size) {
        size_t write_to_end = buf->size - buf->widx;
        if (write_to_end) {
            memcpy(buf->buffer + buf->widx, buffer, write_to_end);
            buffer += write_to_end;
            to_write -= write_to_end;
        }
        buf->widx = 0;
    }

    memcpy(buf->buffer + buf->widx, buffer, to_write);
    buf->widx += to_write;
    return size;
}

size_t ringbuffer_read(ringbuffer_t* buf, size_t size, uint8_t* buffer) {
    size_t avail = ringbuffer_readable(buf);
    if (avail < size) {
        size = avail;
    }

    size_t to_read = size;
    if (buf->ridx + size >= buf->size) {
        size_t read_to_end = buf->size - buf->ridx;
        if (read_to_end) {
            memcpy(buffer, buf->buffer + buf->ridx, read_to_end);
            buffer += read_to_end;
            to_read -= read_to_end;
        }
        buf->ridx = 0;
    }

    memcpy(buffer, buf->buffer + buf->ridx, to_read);
    buf->ridx += to_read;
    return size;
}

void ringbuffer_free(ringbuffer_t* buf) {
    free(buf);
}
