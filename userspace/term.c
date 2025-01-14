/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

/* terminal: terminal host process                                            */

#include <libsystem/iostream.h>
#include <libsystem/error.h>
#include <libsystem/logger.h>
#include <libgraphic/painter.h>

#include <libdevice/textmode.h>
#include <libdevice/framebuffer.h>

#include <libconsole/vtconsole.h>

static iostream_t *terminal_fifo = NULL;

/* --- Textmode terminal ---------------------------------------------------- */

static iostream_t *textmode_device = NULL;
static textmode_info_t textmode_info;
static ushort *textmode_buffer = NULL;

static byte textmode_colors[] = {
    [VTCOLOR_BLACK] = TEXTMODE_COLOR_BLACK,
    [VTCOLOR_RED] = TEXTMODE_COLOR_RED,
    [VTCOLOR_GREEN] = TEXTMODE_COLOR_GREEN,
    [VTCOLOR_YELLOW] = TEXTMODE_COLOR_BROWN,
    [VTCOLOR_BLUE] = TEXTMODE_COLOR_BLUE,
    [VTCOLOR_MAGENTA] = TEXTMODE_COLOR_MAGENTA,
    [VTCOLOR_CYAN] = TEXTMODE_COLOR_CYAN,
    [VTCOLOR_GREY] = TEXTMODE_COLOR_LIGHT_GRAY,
};

static byte textmode_brightcolors[] = {
    [VTCOLOR_BLACK] = TEXTMODE_COLOR_GRAY,
    [VTCOLOR_RED] = TEXTMODE_COLOR_LIGHT_RED,
    [VTCOLOR_GREEN] = TEXTMODE_COLOR_LIGHT_GREEN,
    [VTCOLOR_YELLOW] = TEXTMODE_COLOR_LIGHT_YELLOW,
    [VTCOLOR_BLUE] = TEXTMODE_COLOR_LIGHT_BLUE,
    [VTCOLOR_MAGENTA] = TEXTMODE_COLOR_LIGHT_MAGENTA,
    [VTCOLOR_CYAN] = TEXTMODE_COLOR_LIGHT_CYAN,
    [VTCOLOR_GREY] = TEXTMODE_COLOR_WHITE,
};

void textmode_paint_callback(vtconsole_t *vtc, vtcell_t *vtc_cell, int x, int y)
{
    UNUSED(vtc);

    if (vtc_cell->attr.bright)
    {
        textmode_buffer[x + y * textmode_info.width] = TEXTMODE_ENTRY(vtc_cell->c, textmode_brightcolors[vtc_cell->attr.fg], textmode_colors[vtc_cell->attr.bg]);
    }
    else
    {
        textmode_buffer[x + y * textmode_info.width] = TEXTMODE_ENTRY(vtc_cell->c, textmode_colors[vtc_cell->attr.fg], textmode_colors[vtc_cell->attr.bg]);
    }
}

void textmode_cursor_move_callback(vtconsole_t *vtc, vtcursor_t *cur)
{
    UNUSED(vtc);

    textmode_info.cursor_x = cur->x;
    textmode_info.cursor_y = cur->y;
}

vtconsole_t *terminal_create_textmode_console(void)
{
    textmode_device = iostream_open(TEXTMODE_DEVICE, IOSTREAM_WRITE);

    if (textmode_device == NULL)
    {
        error_print("Failled to open textmode device file");
        return NULL;
    }

    iostream_set_write_buffer_mode(textmode_device, IOSTREAM_BUFFERED_NONE);

    if (iostream_call(textmode_device, TEXTMODE_CALL_GET_INFO, &textmode_info))
    {
        error_print("Failled to get textmode info");
        return NULL;
    }

    vtconsole_t *vtc = vtconsole(80, 25, textmode_paint_callback, textmode_cursor_move_callback);
    textmode_buffer = malloc(textmode_info.width * textmode_info.height * sizeof(ushort));

    return vtc;
}

/* --- Framebuffer terminal ------------------------------------------------- */

point_t char_size = (point_t){9, 24};

static font_t *mono_font = NULL;
static iostream_t *framebuffer_device = NULL;
static point_t framebuffer_cursor = point_zero;

static bitmap_t *framebuffer = NULL;
static painter_t *paint = NULL;

static color_t framebuffer_colors[] = {
    [VTCOLOR_BLACK] = COLOR(0x212121),
    [VTCOLOR_RED] = COLOR(0xb71c1c),
    [VTCOLOR_GREEN] = COLOR(0x1b5e20),
    [VTCOLOR_YELLOW] = COLOR(0xf57f17),
    [VTCOLOR_BLUE] = COLOR(0x0d47a1),
    [VTCOLOR_MAGENTA] = COLOR(0x880e4f),
    [VTCOLOR_CYAN] = COLOR(0x006064),
    [VTCOLOR_GREY] = COLOR(0x607d8b),
};

static color_t framebuffer_brightcolors[] = {
    [VTCOLOR_BLACK] = COLOR(0x9e9e9e),
    [VTCOLOR_RED] = COLOR(0xf44336),
    [VTCOLOR_GREEN] = COLOR(0x4caf50),
    [VTCOLOR_YELLOW] = COLOR(0xffeb3b),
    [VTCOLOR_BLUE] = COLOR(0x2196f3),
    [VTCOLOR_MAGENTA] = COLOR(0xe91e63),
    [VTCOLOR_CYAN] = COLOR(0x00bcd4),
    [VTCOLOR_GREY] = COLOR(0xeceff1),
};


void framebuffer_cursor_move_callback(vtconsole_t *vtc, vtcursor_t *cur)
{
    UNUSED(vtc);

    framebuffer_cursor = (point_t){cur->x, cur->y};
}

vtconsole_t *terminal_create_framebuffer_console(void)
{
    logger_info("Creating graphic context...");

    framebuffer_mode_info_t mode_info = {true, 800, 600};

    framebuffer_device = iostream_open(FRAMEBUFFER_DEVICE, IOSTREAM_READ);

    if (framebuffer_device == NULL)
    {
        error_print("Failled to open " FRAMEBUFFER_DEVICE);

        return NULL;
    }

    logger_info("Framebuffer opened");

    if (iostream_call(framebuffer_device, FRAMEBUFFER_CALL_SET_MODE, &mode_info) < 0)
    {
        error_print("Ioctl to " FRAMEBUFFER_DEVICE " failled");

        return NULL;
    }

    logger_info("Framebuffer mode set");

    framebuffer = bitmap(800, 600);

    paint = painter(framebuffer);

    painter_clear_rect(paint, bitmap_bound(framebuffer), framebuffer_colors[VTCOLOR_BLACK]);

    mono_font = font("mono");

    return vtconsole(mode_info.width / char_size.X, mode_info.height / char_size.Y, NULL, framebuffer_cursor_move_callback);
}

void paint_repaint_dirty(vtconsole_t *console, painter_t* paint)
{
    int repainted_cell = 0;
    bool has_bound = false;
    rectangle_t repaint_bound = {0};

    for (int y = 0; y < console->height; y++)
    {
        for (int x = 0; x < console->width; x++)
        {
            vtcell_t *cell = vtconsole_cell(console, x, y);

            if (cell->is_dirty == true)
            {
                repainted_cell++;

                color_t fgc = cell->attr.bright
                                ? framebuffer_brightcolors[cell->attr.fg]
                                : framebuffer_colors[cell->attr.fg];

                point_t pos = (point_t){x * char_size.X, y * (int)(char_size.Y)};
                point_t siz = (point_t){char_size.X, (char_size.Y)};

                rectangle_t rect;
                rect.position = pos;
                rect.size = siz;

                painter_clear_rect(paint, rect, framebuffer_colors[cell->attr.bg]);
                if (cell->c != ' ')
                {
                    painter_draw_glyph(paint, mono_font, font_glyph(mono_font, cell->c), point_add(pos, (point_t){0, 16}), 16, fgc);
                }
                
                cell->is_dirty = false;

                if (!has_bound)
                {
                    has_bound = true;
                    repaint_bound = rect;
                }
                else
                {
                    repaint_bound = rectangle_merge(repaint_bound, rect);
                }
            }
        }
    }

    // painter_draw_rect(paint, repaint_bound, COLOR_RED);
    // logger_info("Repainted %dcells %dx%d", repainted_cell, repaint_bound.X, repaint_bound.Y);

    framebuffer_region_t reg;
    reg.bound = repaint_bound;
    reg.src = framebuffer->buffer;
    iostream_call(framebuffer_device, FRAMEBUFFER_CALL_BLITREGION, &reg);
}

/* --- Terminal read/write loop --------------------------------------------- */

int main(int argc, char const *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    terminal_fifo = iostream_open("/dev/term", IOSTREAM_READ);

    if (terminal_fifo == NULL)
    {
        error_print("Failled to open terminal fifo file");
        return -1;
    }

    iostream_set_read_buffer_mode(terminal_fifo, IOSTREAM_BUFFERED_NONE);

    vtconsole_t *vtc = NULL;
    bool is_framebuffer = true;
    vtc = terminal_create_framebuffer_console();

    if (vtc == NULL)
    {
        vtc = terminal_create_textmode_console();

        if (vtc == NULL)
        {
            iostream_close(terminal_fifo);

            return -1;
        }
        else
        {
            is_framebuffer = false;
        }
    }

    bool do_exit = false;

    do
    {
#define READ_BUFFER_SIZE 512
        char buffer[READ_BUFFER_SIZE];
        int size = iostream_read(terminal_fifo, buffer, READ_BUFFER_SIZE);

        vtconsole_write(vtc, buffer, size);

        if (!is_framebuffer)
        {
            iostream_call(textmode_device, TEXTMODE_CALL_SET_INFO, &textmode_info);
            iostream_write(textmode_device, textmode_buffer, textmode_info.width * textmode_info.height * sizeof(ushort));
        }
        else
        {
            paint_repaint_dirty(vtc, paint);
        }
    } while (!do_exit);

    vtconsole_delete(vtc);

    return 0;
}
