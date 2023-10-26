#pragma once

typedef volatile struct {
    volatile int latch[1];
} spin_lock_t;

#define spin_init(lock) { (lock).owner = 0; (lock).latch[0] = 0; }

#define spin_lock(lock) { while (__sync_lock_test_and_set((lock).latch, 0x01)); }
#define spin_unlock(lock) { __sync_lock_release((lock).latch); }