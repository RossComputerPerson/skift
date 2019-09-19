#ifndef __LIBDESKTOP_IPC_H_
#define __LIBDESKTOP_IPC_H_ 1

typedef enum {
	LIBDESKTOP_APP_REGISTER,
	LIBDESKTOP_APP_UNREGISTER,
	LIBDESKTOP_APP_GETWINS,

	LIBDESKTOP_WIN_CREATE,
	LIBDESKTOP_WIN_DESTROY,
	LIBDESKTOP_WIN_SETPROP,
	LIBDESKTOP_WIN_GETPROP,
	LIBDESKTOP_WIN_UPDATE,
	LIBDESKTOP_WIN_SYNC
} libdesktop_ipc_opcode;

#ifdef __LIBDESKTOP_DEV
#include <libsystem/messaging.h>

#define LIBDESKTOP_CLIENT_CONNECT MSGLABEL("libdesktop", "client", "connect")
#define LIBDESKTOP_CLIENT_DISCONNECT MSGLABEL("libdesktop", "client", "disconnect")
#define LIBDESKTOP_CLIENT_EXEC MSGLABEL("libdesktop", "client", "exec")

typedef struct {
	libdesktop_ipc_opcode op;
	char* appid;
	int argc;
	void** argv;
} libdesktop_packet;
#endif

int libdesktop_client_connect(const char* appid);
int libdesktop_client_disconnect(const char* appid);
int libdesktop_client_exec(const char* appid, libdesktop_ipc_opcode op, int* iargc, void*** iargv, int argc, ...);

#endif
