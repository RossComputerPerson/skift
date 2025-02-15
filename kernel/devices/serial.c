/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/runtime.h>
#include <libsystem/atomic.h>

#include "system.h"
#include "processor.h"
#include "filesystem.h"

#include "serial.h"

#define PORT_COM1 0x3f8

static void wait_read()
{
    while ((in8(PORT_COM1 + 5) & 1) == 0)
    { /* do nothing */
    }
}

static void wait_write()
{
    while ((in8(PORT_COM1 + 5) & 0x20) == 0)
    { /* do nothing */
    }
}

/* --- Device driver -------------------------------------------------------- */

static int serial_device_write(stream_t *s, const void *buffer, uint size)
{
    UNUSED(s);

    return serial_write(buffer, size);
}

static int serial_device_read(stream_t *s, void *buffer, uint size)
{
    UNUSED(s);

    return serial_read(buffer, size);
}

/* --- Public functions ----------------------------------------------------- */

void serial_setup()
{
    // See: https://wiki.osdev.org/Serial_Ports
    out8(PORT_COM1 + 1, 0x00);
    out8(PORT_COM1 + 3, 0x80);
    out8(PORT_COM1 + 0, 0x03);
    out8(PORT_COM1 + 1, 0x00);
    out8(PORT_COM1 + 3, 0x03);
    out8(PORT_COM1 + 2, 0xC7);
    out8(PORT_COM1 + 4, 0x0B);

    device_t serial_device = {
        .read = serial_device_read,
        .write = serial_device_write,
    };

    FILESYSTEM_MKDEV("/dev/serial", serial_device);
}

char serial_getc()
{
    wait_read();
    return in8(PORT_COM1);
}

void serial_putc(char c)
{
    wait_write();
    out8(PORT_COM1, c);
}

int serial_read(char *buffer, uint size)
{
    for (uint i = 0; i < size; i++)
    {
        buffer[i] = serial_getc();

        if (buffer[i] == '\n')
        {
            return i + 1;
        }
    }

    return size;
}

int serial_write(const char *buffer, uint size)
{
    atomic_begin();
    
    for (uint i = 0; i < size; i++)
    {
        serial_putc(buffer[i]);
    }

    atomic_end();
    
    return size;
}