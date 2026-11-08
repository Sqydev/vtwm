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

#include "../actions/actions.h"
#include "../socket/socket.h"
#include "../../defines.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void DestroyIpcClient(Ipc* ipcServer, IpcClient* ipcClient) {
	if(!ipcServer || !ipcClient) {
		return;
	}

	IpcClient** clientLink = &ipcServer->clients;

	while(*clientLink && *clientLink != ipcClient) {
		clientLink = &(*clientLink)->next;
	}

	if(*clientLink == ipcClient) {
		*clientLink = ipcClient->next;
	}

	if(ipcClient->source) {
		wl_event_source_remove(ipcClient->source);
	}

	close(ipcClient->fd);
	free(ipcClient);
}

int HandleIpcClientEvent(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)mask;

	IpcClient* ipcClient = data;
	Ipc* ipcServer = ipcClient->ipc;

	char commandBuffer[IPC_COMMAND_MAX];

	ssize_t receivedLength = ReceiveIpcText(ipcClient->fd, commandBuffer, sizeof(commandBuffer));

	if(receivedLength > 0) {
		size_t commandLength = (size_t)receivedLength;

		while(commandLength > 0 && (commandBuffer[commandLength - 1] == '\n' || commandBuffer[commandLength - 1] == '\r')) {
			commandBuffer[--commandLength] = '\0';
		}

		const char* responseStatus = HandleIpcCommand(commandBuffer);

		char responseBuffer[IPC_RESPONSE_MAX];

		int responseLength = snprintf(responseBuffer, sizeof(responseBuffer), "%s\n", responseStatus);
		if(responseLength > 0 && (size_t)responseLength < sizeof(responseBuffer)) {
			SendIpcText(ipcClient->fd, responseBuffer);
		}
	}

	DestroyIpcClient(ipcServer, ipcClient);

	return 0;
}
