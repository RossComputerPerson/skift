#pragma once

/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/runtime.h>

typedef struct
{
    bool locked;
    int holder;
    const char *name;
} lock_t;

void __lock_init(lock_t *lock, const char *name);

void __lock_acquire(lock_t *lock);

void __lock_acquire_by(lock_t *lock, int holder);

void __lock_release(lock_t *lock, const char *file, const char *function, int line);

bool __lock_try_acquire(lock_t *lock);

void __lock_assert(lock_t *lock, const char *file, const char *function, int line);

#define lock_init(lock) __lock_init(&lock, #lock)

#define lock_acquire(lock) __lock_acquire(&lock)

#define lock_acquire_by(lock, __holder) __lock_acquire_by(&lock, __holder)

#define lock_try_acquire(lock) __lock_try_acquire(&lock)

#define lock_release(lock) __lock_release(&lock, __FILE__, __FUNCTION__, __LINE__)

#define lock_assert(lock) __lock_assert(&lock, __FILE__, __FUNCTION__, __LINE__)

#define lock_is_acquire(__lock) ((&__lock)->locked)

#define LOCK(lock, code)       \
    do                         \
    {                          \
        __lock_acquire(&lock); \
        code;                  \
        __lock_release(&lock); \
    } while (0);
