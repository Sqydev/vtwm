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
