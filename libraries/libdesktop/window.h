#ifndef __LIBDESKTOP_WINDOW_H_
#define __LIBDESKTOP_WINDOW_H_ 1

#define LIBDESKTOP_WIN_FLAG_NODECOR (1 << 0)
#define LIBDESKTOP_WIN_FLAG_SELFONLY (1 << 1)
#define LIBDESKTOP_WIN_FLAG_FULLSCREEN (1 << 2)
#define LIBDESKTOP_WIN_FLAG_MAXIMIZE (1 << 3)
#define LIBDESKTOP_WIN_FLAG_MINIMIZE (1 << 4)
#define LIBDESKTOP_WIN_FLAG_SHADE (1 << 5)
#define LIBDESKTOP_WIN_FLAG_LOCKPOS (1 << 6)
#define LIBDESKTOP_WIN_FLAG_LOCKSIZE (1 << 7)

typedef struct {
	unsigned int id;
	unsigned long flags;

	char* title;
	char* appid;

	unsigned long x;
	unsigned long y;
	unsigned long width;
	unsigned long height;
} libdesktop_window;

int libdesktop_window_create(libdesktop_window** win, const char* appid, const char* title, unsigned long flags, unsigned long x, unsigned long y, unsigned long width, unsigned long height);
int libdesktop_window_destroy(libdesktop_window** win);

#endif
