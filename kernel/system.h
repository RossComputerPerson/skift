#pragma once

/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/runtime.h>
#include <libsystem/logger.h>

#include "processor.h"

void __attribute__((noreturn)) __panic(const char *package, const char *file, const char *function, const int line, processor_context_t *context, const char *message, ...);

#define HANG   \
    while (1)  \
    {          \
        hlt(); \
    }

#define STOP   \
    while (1)  \
    {          \
        cli(); \
        hlt(); \
    }

#define PANIC(x...) __panic(__PACKAGE__, __FILE__, __FUNCTION__, __LINE__, NULL, x)

#define CPANIC(ctx, x...) __panic(__PACKAGE__, __FILE__, __FUNCTION__, __LINE__, ctx, x)

#define setup(x, arg...)                     \
    {                                        \
        logger_info("Setting up " #x "..."); \
        x##_setup(arg);                      \
    }

#define KERNEL_VERSION __kernel_version_format, __kernel_version_major, __kernel_version_minor, __kernel_version_patch, __kernel_version_codename

#define KERNEL_UNAME __kernel_uname_format, __kernel_name, __kernel_version_major, __kernel_version_minor, __kernel_version_patch, __kernel_version_codename

extern char *__kernel_name;
extern int __kernel_version_major;
extern int __kernel_version_minor;
extern int __kernel_version_patch;
extern char *__kernel_version_codename;

extern char *__kernel_version_format;
extern char *__kernel_uname_format;
