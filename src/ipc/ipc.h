#ifndef VTWM_IPC_H
#define VTWM_IPC_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include <wayland-server-core.h>

typedef struct IpcClient IpcClient;

typedef struct Ipc {
	int listenFd;

	char path[PATH_MAX];

	struct wl_event_source* source;
	struct wl_display* display;

	IpcClient* clients;
} Ipc;

struct IpcClient {
	int fd;

	Ipc* ipc;

	struct wl_event_source* source;

	IpcClient* next;
};

bool InitIPC(struct wl_display* display);
void FinishIPC(void);

#endif
