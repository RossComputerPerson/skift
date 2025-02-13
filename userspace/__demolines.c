/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/iostream.h>
#include <libsystem/cstring.h>
#include <libsystem/error.h>
#include <libgraphic/bitmap.h>
#include <libgraphic/painter.h>
#include <libsystem/logger.h>
#include <libsystem/assert.h>

#include <libdevice/framebuffer.h>

framebuffer_mode_info_t mode_info = {true, 800,  600};

typedef struct
{   
    point_t start;
    point_t finish;
    color_t color;
} a_line_t;


int main(int argc, char **argv)
{
    UNUSED(argc); UNUSED(argv);

    iostream_t *framebuffer_device = iostream_open(FRAMEBUFFER_DEVICE, IOSTREAM_READ);

    if (framebuffer_device == NULL)
    {
        error_print("Failled to open " FRAMEBUFFER_DEVICE);
        return -1;
    }

    if (iostream_call(framebuffer_device, FRAMEBUFFER_CALL_SET_MODE, &mode_info) < 0)
    {
        error_print("Ioctl to " FRAMEBUFFER_DEVICE " failled");
        return -1;
    }

    iostream_t *random_device = iostream_open("/dev/random", IOSTREAM_READ);

    if (random_device == NULL)
    {
        error_print("Failled to open random");
        return -1;
    }

    bitmap_t* fb = bitmap(800, 600);

    assert(fb);

    painter_t* paint = painter(fb);

    assert(paint);

    do
    {
        a_line_t line;
        iostream_read(random_device, &line, sizeof(line));

        line.start.X  = abs((int)line.start.X % 800);
        line.start.Y  = abs((int)line.start.Y % 600);
        line.finish.X = abs((int)line.finish.X % 800);
        line.finish.Y = abs((int)line.finish.Y % 600);

        painter_draw_line(paint, line.start, line.finish, line.color);
        
        framebuffer_region_t reg;
        reg.src = fb->buffer;
        reg.bound.X = min(line.start.X, line.finish.X);
        reg.bound.Y = min(line.start.Y, line.finish.Y);
        reg.bound.width = max(line.start.X, line.finish.X) - reg.bound.X;
        reg.bound.height = max(line.start.Y, line.finish.Y) - reg.bound.Y;

        iostream_call(framebuffer_device, FRAMEBUFFER_CALL_BLITREGION, &reg);
    } while(true);

    return 0;
}