#define __LIBDESKTOP_DEV 1
#include <libdesktop/ipc.h>
#include <libdesktop/window.h>

int libdesktop_window_create(libdesktop_window** win, const char* appid, const char* title, unsigned long flags, unsigned long x, unsigned long y, unsigned long width, unsigned long height) {
	void** argv;
	int argc;
	if (libdesktop_client_exec(appid, LIBDESKTOP_WIN_CREATE, &argc, &argv, 6, title, flags, x, y, width, height) == -1) return -1;
	memset(win, 0, sizeof(libdesktop_window));
	(*win)->id = (unsigned int)argv[0];
	(*win)->x = (unsigned long)argv[1];
	(*win)->y = (unsigned long)argv[2];
	(*win)->width = (unsigned long)argv[3];
	(*win)->height = (unsigned long)argv[4];
	(*win)->flags = flags;
	return 0;
}
