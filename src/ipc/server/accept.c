#include "./server.h"

#include "../socket/socket.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wlr/util/log.h>

int HandleAccept(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)mask;

	Ipc* ipcServer = data;

	for(;;) {
		int clientSocketFd = accept(ipcServer->listenFd, NULL, NULL);
		if(clientSocketFd < 0) {
			if(errno == EINTR) {
				continue;
			}

			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			wlr_log(WLR_ERROR, "IPC accept failed: %s", strerror(errno));
			break;
		}

		if(SetNonBlockingIpc(clientSocketFd) < 0) {
			wlr_log(WLR_ERROR, "Cannot configure IPC client: %s", strerror(errno));
			close(clientSocketFd);
			continue;
		}

		IpcClient* ipcClient = calloc(1, sizeof(*ipcClient));
		if(!ipcClient) {
			close(clientSocketFd);
			continue;
		}

		ipcClient->fd = clientSocketFd;
		ipcClient->ipc = ipcServer;

		struct wl_event_loop* eventLoop = wl_display_get_event_loop(ipcServer->display);

		ipcClient->source = wl_event_loop_add_fd(eventLoop, clientSocketFd, WL_EVENT_READABLE, HandleIpcClientEvent, ipcClient);
		if(!ipcClient->source) {
			close(clientSocketFd);
			free(ipcClient);
			continue;
		}

		ipcClient->next = ipcServer->clients;
		ipcServer->clients = ipcClient;
	}

	return 0;
}
