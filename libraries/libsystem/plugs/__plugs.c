/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

/* plugs.c: Plugs functions for using the skift Framework in the userspace.   */

#include <libsystem/__plugs__.h>
#include <libsystem/cstring.h>
#include <libsystem/lock.h>
#include <libsystem/logger.h>
#include <libsystem/process.h>

lock_t memlock;
lock_t loglock;

iostream_t *in_stream;
iostream_t *out_stream;
iostream_t *err_stream;
iostream_t *log_stream;

extern void _init();
extern void _fini();

void __plug_init(void)
{
    lock_init(memlock);
    lock_init(loglock);

    // Open io stream
    in_stream = NULL; // FIXME: no stdin,
    out_stream = iostream_open("/dev/term", IOSTREAM_WRITE | IOSTREAM_BUFFERED_WRITE);
    err_stream = iostream_open("/dev/term", IOSTREAM_WRITE | IOSTREAM_BUFFERED_WRITE);
    log_stream = iostream_open("/dev/serial", IOSTREAM_WRITE | IOSTREAM_BUFFERED_WRITE);

    _init();
}

void __plug_fini(int exit_code)
{
    _fini();

    iostream_flush(out_stream);
    iostream_flush(err_stream);
    iostream_flush(log_stream);

    process_exit(exit_code);
}

void __plug_assert_failed(const char *expr, const char *file, const char *function, int line)
{
    logger_fatal("Assert failed: %s in %s:%s() ln%d!", expr, file, function, line);
    process_exit(-1);
}

void __plug_lock_assert_failed(lock_t *lock, const char *file, const char *function, int line)
{
    logger_fatal("Lock assert failed: %s hold by %d in %s:%s() ln%d!", lock->name, lock->holder, file, function, line);
    process_exit(-1);
}

int __plug_logger_lock()
{
    lock_acquire(loglock);
    return 0;
}

int __plug_logger_unlock()
{
    lock_release(loglock);
    return 0;
}

int __plug_memalloc_lock()
{
    lock_acquire(memlock);
    return 0;
}

int __plug_memalloc_unlock()
{
    lock_release(memlock);
    return 0;
}

void *__plug_memalloc_alloc(uint size)
{
    uint addr = process_alloc(size);
    return (void *)addr;
}

int __plug_memalloc_free(void *memory, uint size)
{
    return process_free((unsigned int)memory, size);
}