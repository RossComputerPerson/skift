#define __LIBDESKTOP_DEV
#include <libdesktop/ipc.h>
#include <libdesktop/prop.h>
#include <libsystem/error.h>
#include <libsystem/messaging.h>

int libdesktop_client_connect(const char* appid) {
	message_t msg = message(LIBDESKTOP_CLIENT_CONNECT, -1);
	message_set_payload_ptr(msg, appid, strlen(appid));
	return messaging_broadcast(LIBDESKTOP_CLIENT_CONNECT, &msg);
}

int libdesktop_client_disconnect(const char* appid) {
	message_t msg = message(LIBDESKTOP_CLIENT_DISCONNECT, -1);
	message_set_payload_ptr(msg, appid, strlen(appid));
	return messaging_broadcast(LIBDESKTOP_CLIENT_DISCONNECT, &msg);
}

int libdesktop_client_exec(const char* appid, libdesktop_ipc_opcode op, int* iargc, void*** iargv, int argc, ...) {
	libdesktop_packet* pkt = malloc(sizeof(libdesktop_packet));
	if (pkt == NULL) {
		error_set(-ERR_CANNOT_ALLOCATE_MEMORY);
		return -1;
	}
	pkt->appid = (char*)appid;
	pkt->op = op;
	pkt->argc = argc;
	if ((pkt->argv = malloc(sizeof(void*) * pkt->argc)) == NULL) {
		free(pkt);
		error_set(-ERR_CANNOT_ALLOCATE_MEMORY);
		return -1;
	}
	va_list ap;
	va_start(ap, argc);
	for (int i = 0; i < argc; i++) pkt->argv[i] = va_arg(ap, void*);
	va_end(ap);
	message_t msg = message(LIBDESKTOP_CLIENT_EXEC, 1);
	message_set_payload_ptr(msg, pkt, sizeof(libdesktop_packet));
	message_t res;
	free(pkt);
	if (messaging_request(&msg, &res, 6000) == -1) return -1;
	pkt = (libdesktop_packet*)message_payload(res);
	*iargc = pkt->argc;
	if ((*iargv = malloc(sizeof(void*) * pkt->argc)) == NULL) {
		error_set(-ERR_CANNOT_ALLOCATE_MEMORY);
		return -1;
	}
	memcpy(*iargv, pkt->argv, sizeof(void*) * pkt->argc);
	return 0;
}

int libdesktop_client_atom(const char* name, unsigned long* id) {
	message_t msg = message(LIBDESKTOP_CLIENT_ATOM, -1);
	message_set_payload_ptr(msg, name, strlen(name));
	message_t res;
	if (messaging_request(&msg, &res, 6000) == -1) return -1;
	*id = (unsigned long)message_payload(res);
	return 0;
}
