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
