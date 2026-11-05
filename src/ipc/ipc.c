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

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wlr/util/log.h>

#include "../config/config.h"
#include "../coredata.h"

struct IpcClient {
	int fd;
	Ipc* ipc;
	struct wl_event_source* source;
	IpcClient* next;
};

const char* HandleIpcCommand(const char* command) {
	if(!command) {
		return "unknown";
	}

	while(*command == ' ' || *command == '\t') {
		command++;
	}

	size_t length = strlen(command);

	while(length > 0 && (command[length - 1] == ' ' || command[length - 1] == '\t')) {
		length--;
	}

	if(length == 6 && strncmp(command, "reload", length) == 0) {
		wlr_log(WLR_DEBUG, "Got config reload command");

		if(!ReloadConfig()) {
			return "error";
		}

		UpdateCursorConfig();
		UpdateKeyboardConfig();

		return "ok";
	}

	return "unknown";
}

void DestroyIpcClient(Ipc* ipc, IpcClient* client) {
	if(!ipc || !client) {
		return;
	}

	IpcClient** link = &ipc->clients;

	while(*link && *link != client) {
		link = &(*link)->next;
	}

	if(*link == client) {
		*link = client->next;
	}

	if(client->source) {
		wl_event_source_remove(client->source);
	}

	close(client->fd);
	free(client);
}

int HandleIpcClientEvent(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)mask;

	IpcClient* client = data;
	Ipc* ipc = client->ipc;
	char command[IPC_COMMAND_MAX];
	ssize_t received = ReceiveIpcText(client->fd, command, sizeof(command));

	if(received > 0) {
		size_t length = (size_t)received;

		while(length > 0 && (command[length - 1] == '\n' || command[length - 1] == '\r')) {
			command[--length] = '\0';
		}

		const char* status = HandleIpcCommand(command);
		char response[IPC_RESPONSE_MAX];
		int written = snprintf(response, sizeof(response), "%s\n", status);

		if(written > 0 && (size_t)written < sizeof(response)) {
			SendIpcText(client->fd, response);
		}
	}

	DestroyIpcClient(ipc, client);

	return 0;
}

int IpcHandleAccept(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)mask;

	Ipc* ipc = data;

	for(;;) {
		int clientFd = accept(ipc->listenFd, NULL, NULL);

		if(clientFd < 0) {
			if(errno == EINTR) {
				continue;
			}

			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			wlr_log(WLR_ERROR, "IPC: accept failed: %s", strerror(errno));
			break;
		}

		if(SetNonBlockingIpc(clientFd) < 0) {
			wlr_log(WLR_ERROR, "IPC: cannot configure client: %s", strerror(errno));
			close(clientFd);
			continue;
		}

		IpcClient* client = calloc(1, sizeof(*client));

		if(!client) {
			close(clientFd);
			continue;
		}

		client->fd = clientFd;
		client->ipc = ipc;

		struct wl_event_loop* loop = wl_display_get_event_loop(ipc->display);

		client->source = wl_event_loop_add_fd(loop, clientFd, WL_EVENT_READABLE, HandleIpcClientEvent, client);

		if(!client->source) {
			close(clientFd);
			free(client);
			continue;
		}

		client->next = ipc->clients;
		ipc->clients = client;
	}

	return 0;
}

bool InitIPC(struct wl_display* display) {
	Ipc* ipc = &IPC;

	memset(ipc, 0, sizeof(*ipc));
	ipc->listenFd = -1;
	ipc->display = display;

	if(!display) {
		wlr_log(WLR_ERROR, "IPC: cannot initialize without a display");
		return false;
	}

	if(!GetIpcServerPath(ipc->path, sizeof(ipc->path))) {
		wlr_log(WLR_ERROR, "IPC: cannot determine socket path: %s", strerror(errno));
		return false;
	}

	int listenFd = CreateIpcListener(ipc->path);

	if(listenFd < 0) {
		wlr_log(WLR_ERROR, "IPC: cannot create %s: %s", ipc->path, strerror(errno));
		ipc->path[0] = '\0';

		return false;
	}

	ipc->listenFd = listenFd;

	struct wl_event_loop* loop = wl_display_get_event_loop(display);

	ipc->source = wl_event_loop_add_fd(loop, listenFd, WL_EVENT_READABLE, IpcHandleAccept, ipc);

	if(!ipc->source) {
		int error = errno;

		close(listenFd);
		unlink(ipc->path);
		ipc->listenFd = -1;
		ipc->path[0] = '\0';

		wlr_log(WLR_ERROR, "IPC: cannot add listener: %s", strerror(error));

		return false;
	}

	wlr_log(WLR_INFO, "IPC: listening on %s", ipc->path);

	return true;
}

void FinishIPC(void) {
	Ipc* ipc = &IPC;

	IpcClient* client = ipc->clients;

	while(client) {
		IpcClient* next = client->next;

		if(client->source) {
			wl_event_source_remove(client->source);
		}

		close(client->fd);
		free(client);

		client = next;
	}

	ipc->clients = NULL;

	if(ipc->source) {
		wl_event_source_remove(ipc->source);
		ipc->source = NULL;
	}

	if(ipc->listenFd >= 0) {
		close(ipc->listenFd);
		ipc->listenFd = -1;
	}

	if(ipc->path[0] != '\0') {
		unlink(ipc->path);
		ipc->path[0] = '\0';
	}

	ipc->display = NULL;
}
