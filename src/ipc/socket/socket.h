#ifndef VTWM_IPC_SOCKET_H
#define VTWM_IPC_SOCKET_H

#include "../ipc.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

bool GetIpcRuntimeDir(char* directoryPath, size_t bufferSize);
bool GetIpcServerPath(char* socketPath, size_t bufferSize);

bool MakeIpcAddress(const char* socketPath, struct sockaddr_un* socketAddress, socklen_t* socketAddressLength);

int CreateIpcListener(const char* socketPath);
int IpcConnect(const char* socketPath);

int SetNonBlockingIpc(int socketFd);
int SetIpcTimeout(int socketFd, int milliseconds);

ssize_t SendIpcText(int socketFd, const char* text);
ssize_t ReceiveIpcText(int socketFd, char* buffer, size_t bufferSize);

#endif
