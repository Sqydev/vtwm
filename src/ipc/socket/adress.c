#include "./socket.h"

#include "../../defines.h"
#include "../../utils/utils.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

bool GetIpcRuntimeDir(char* directoryPath, size_t bufferSize) {
	if(!directoryPath || bufferSize == 0) {
		errno = EINVAL;
		return false;
	}

	char* runtimeDirectory;
	if(!(runtimeDirectory = GetXdgRuntimePath())) {
		return false;
	}

	int writtenLength = snprintf(directoryPath, bufferSize, "%s/%s", runtimeDirectory, IPC_RUNTIME_SOCKET_DIR_NAME);
	if(writtenLength <= 0 || (size_t)writtenLength >= bufferSize) {
		errno = ENAMETOOLONG;
		return false;
	}

	return true;
}

bool GetIpcServerPath(char* socketPath, size_t bufferSize) {
	if(!socketPath || bufferSize == 0) {
		errno = EINVAL;
		return false;
	}

	char dirPath[PATH_MAX];
	if(!GetIpcRuntimeDir(dirPath, bufferSize)) {
		return false;
	}

	if(mkdir(dirPath, 0700) != 0 && errno != EEXIST) {
		return false;
	}

	int writtenLength = snprintf(socketPath, bufferSize, "%s/%s%d%s", dirPath, IPC_SOCKET_PREFIX, (int)getpid(), IPC_SOCKET_SUFFIX);
	if(writtenLength <= 0 || (size_t)writtenLength >= bufferSize) {
		errno = ENAMETOOLONG;
		return false;
	}

	return true;
}

bool MakeIpcAddress(const char* socketPath, struct sockaddr_un* socketAddress, socklen_t* socketAddressLength) {
	if(!socketPath || !socketAddress || !socketAddressLength) {
		errno = EINVAL;
		return false;
	}

	if(strlen(socketPath) >= sizeof(socketAddress->sun_path)) {
		errno = ENAMETOOLONG;
		return false;
	}

	memset(socketAddress, 0, sizeof(*socketAddress));
	socketAddress->sun_family = AF_UNIX; // Local socket
	strcpy(socketAddress->sun_path, socketPath);

	// Hacking to the gate of adress
	*socketAddressLength = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + strlen(socketAddress->sun_path) + 1);

	return true;
}
