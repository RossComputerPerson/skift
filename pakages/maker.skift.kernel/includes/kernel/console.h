#pragma once

#include <skift/generic.h>
#include <skift/list.h>
#include <skift/lock.h>

typedef enum
{
    CSLC_BLACK,
    CSLC_RED,
    CSLC_GREEN,
    CSLC_YELLOW,
    CSLC_BLUE,
    CSLC_MAGENTA,
    CSLC_CYAN,
    CSLC_LIGHT_GREY,

    CSLC_DARK_GREY,
    CSLC_LIGHT_RED,
    CSLC_LIGHT_GREEN,
    CSLC_LIGHT_YELLOW,
    CSLC_LIGHT_BLUE,
    CSLC_LIGHT_MAGENTA,
    CSLC_LIGHT_CYAN,
    CSLC_WHITE,

    CSLC_DEFAULT_FORGROUND,
    CSLC_DEFAULT_BACKGROUND,
} console_color_t;

typedef struct
{
    char c;

    console_color_t fg;
    console_color_t bg;
} console_cell_t;

typedef struct
{
    list_t *cells;
} console_line_t;

typedef enum
{
    CSSTATE_ESC,
    CSSTATE_BRACKET,
    CSSTATE_PARSE,
    CSSTATE_BGCOLOR,
    CSSTATE_FGCOLOR,
    CSSTATE_ENDVAL,
} console_state_t;

typedef struct
{
    lock_t lock;

    console_state_t state;

    list_t *lines;
    console_line_t *current_line;

    console_color_t fg;
    console_color_t bg;

    console_color_t newfg;
    console_color_t newbg;
} console_t;

void console_setup();

void console_print(const char *s);
void console_putchar(char c);

void console_read(const char *s, uint size);
void console_getchar(char c);