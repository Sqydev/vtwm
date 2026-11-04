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

#ifndef VTWM_IPC_H
#define VTWM_IPC_H

#include "../defines.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <wayland-server-core.h>

typedef struct IpcClient IpcClient;

typedef struct Ipc {
	int listenFd;
	char path[PATH_MAX];
	struct wl_event_source* source;
	struct wl_display* display;
	IpcClient* clients;
} Ipc;

bool InitIPC(struct wl_display* display);
void FinishIPC(void);

bool GetIpcServerPath(char* path, size_t size);
bool MakeIpcAddress(const char* path, struct sockaddr_un* addr, socklen_t* length);

int CreateIpcListener(const char* path);
int IpcConnect(const char* path);
int SetNonBlockingIpc(int fd);
int SetIpcTimeout(int fd, int milliseconds);
ssize_t SendIpcText(int fd, const char* text);
ssize_t ReceiveIpcText(int fd, char* buffer, size_t size);

#endif
