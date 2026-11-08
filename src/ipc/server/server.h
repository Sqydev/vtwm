#ifndef VTWM_IPC_SERVER_H
#define VTWM_IPC_SERVER_H

#include "../ipc.h"

void DestroyIpcClient(Ipc* ipcServer, IpcClient* ipcClient);

int HandleIpcClientEvent(int fd, uint32_t mask, void* data);
int HandleAccept(int fd, uint32_t mask, void* data);

#endif
