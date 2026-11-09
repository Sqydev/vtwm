/* 
* Copyright (c) 2025-present W.K. ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

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
