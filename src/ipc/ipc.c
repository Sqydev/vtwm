#include "./ipc.h"

#include "../coredata.h"

#include "./server/server.h"
#include "./socket/socket.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <wlr/util/log.h>

bool InitIPC(struct wl_display* display) {
	memset(&IPC, 0, sizeof(Ipc));

	IPC.listenFd = -1;

	IPC.display = display;
	if(!display) {
		wlr_log(WLR_ERROR, "Cannot initialize IPC without a display");
		return false;
	}

	if(!GetIpcServerPath(IPC.path, sizeof(IPC.path))) {
		wlr_log(WLR_ERROR, "Cannot determine IPC socket path: %s", strerror(errno));
		return false;
	}

	int listenerSocketFd = CreateIpcListener(IPC.path);
	if(listenerSocketFd < 0) {
		wlr_log(WLR_ERROR, "Cannot create %s: %s", IPC.path, strerror(errno));
		IPC.path[0] = '\0';

		return false;
	}

	IPC.listenFd = listenerSocketFd;

	struct wl_event_loop* eventLoop = wl_display_get_event_loop(display);

	IPC.source = wl_event_loop_add_fd(eventLoop, listenerSocketFd, WL_EVENT_READABLE, HandleAccept, &IPC);
	if(!IPC.source) {
		int savedErrno = errno;

		close(listenerSocketFd);
		unlink(IPC.path);
		IPC.listenFd = -1;
		IPC.path[0] = '\0';

		wlr_log(WLR_ERROR, "Cannot add IPC listener(source): %s", strerror(savedErrno));

		return false;
	}

	wlr_log(WLR_INFO, "IPC listening on %s", IPC.path);

	return true;
}

void FinishIPC(void) {
	while(IPC.clients) {
		IpcClient* nextClient = IPC.clients->next;

		if(IPC.clients->source) {
			wl_event_source_remove(IPC.clients->source);
		}

		close(IPC.clients->fd);
		free(IPC.clients);

		IPC.clients = nextClient;
	}

	IPC.clients = NULL;

	if(IPC.source) {
		wl_event_source_remove(IPC.source);
		IPC.source = NULL;
	}

	if(IPC.listenFd >= 0) {
		close(IPC.listenFd);
		IPC.listenFd = -1;
	}

	char runtimeDirectoryPath[PATH_MAX];

	if(IPC.path[0] != '\0') {
		unlink(IPC.path);
		IPC.path[0] = '\0';
	}

	if(GetIpcRuntimeDir(runtimeDirectoryPath, sizeof(runtimeDirectoryPath))) {
		// If there are other sockets it will just fail
		rmdir(runtimeDirectoryPath);
	}

	IPC.display = NULL;
}
