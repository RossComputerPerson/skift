#define __LIBDESKTOP_DEV 1
#include <libdesktop/ipc.h>
#include <libdesktop/window.h>
#include <libsystem/error.h>

int libdesktop_window_create(libdesktop_window** win, const char* appid, const char* title, unsigned long flags, unsigned long x, unsigned long y, unsigned long width, unsigned long height) {
	void** argv;
	int argc;
	if (libdesktop_client_exec(appid, LIBDESKTOP_WIN_CREATE, &argc, &argv, 6, title, flags, x, y, width, height) == -1) return -1;
	memset(*win, 0, sizeof(libdesktop_window));
	(*win)->id = (unsigned int)argv[0];
	(*win)->x = (unsigned long)argv[1];
	(*win)->y = (unsigned long)argv[2];
	(*win)->width = (unsigned long)argv[3];
	(*win)->height = (unsigned long)argv[4];
	(*win)->depth = (int)argv[5];
	(*win)->flags = flags;
	(*win)->appid = (char*)appid;
	if (((*win)->framebuffer = malloc((*win)->depth * ((*win)->width * (*win)->height))) == NULL) {
		error_set(-ERR_CANNOT_ALLOCATE_MEMORY);
		return -1;
	}
	memset((*win)->framebuffer, 0, (*win)->depth * ((*win)->width * (*win)->height));
	return 0;
}

int libdesktop_window_destroy(libdesktop_window** win) {
	free((*win)->framebuffer);
	void** argv;
	int argc;
	return libdesktop_client_exec((*win)->appid, LIBDESKTOP_WIN_DESTROY, &argc, &argv, 1, (*win)->id);
}

int libdesktop_window_getprop(libdesktop_window* win, unsigned long id, size_t* size, void** value) {
	void** argv;
	int argc;
	if (libdesktop_client_exec(win->appid, LIBDESKTOP_WIN_GETPROP, &argc, &argv, 2, win->id, id) == -1) return -1;
	memcpy(value, argv, argc);
        *size = argc;	
	return -1;
}

int libdesktop_window_setprop(libdesktop_window* win, unsigned long id, size_t size, void* value) {
	void** argv;
	int argc;
	return libdesktop_client_exec(win->appid, LIBDESKTOP_WIN_SETPROP, &argc, &argv, 4, win->id, id, size, value);
}

int libdesktop_window_update(libdesktop_window* win) {
	void** argv;
	int argc;
	return libdesktop_client_exec(win->appid, LIBDESKTOP_WIN_UPDATE, &argc, &argv, 5, win->id, win->x, win->y, win->depth, win->width, win->height, win->framebuffer, win->flags);
}

int libdesktop_window_sync(libdesktop_window** win) {
	void** argv;
	int argc;
	if (libdesktop_client_exec((*win)->appid, LIBDESKTOP_WIN_SYNC, &argc, &argv, 1, (*win)->id) == -1) return -1;
	(*win)->x = (unsigned long)argv[0];
	(*win)->y = (unsigned long)argv[1];
	(*win)->width = (unsigned long)argv[2];
	(*win)->height = (unsigned long)argv[3];
	(*win)->depth = (int)argv[4];
	return 0;
}
