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

#include "../utils/utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/time.h>

bool GetIpcServerPath(char* path, size_t size) {
	if(!path || size == 0) {
		errno = EINVAL;
		return false;
	}

	char* runtime;
	if(!(runtime = GetXdgRuntimePath())) {
		return false;
	}

	int written = snprintf(
		path,
		size,
		"%s/%s%d%s",
		runtime,
		IPC_SOCKET_PREFIX,
		(int)getpid(),
		IPC_SOCKET_SUFFIX
	);

	if(written <= 0 || (size_t)written >= size) {
		errno = ENAMETOOLONG;
		return false;
	}

	return true;
}

bool MakeIpcAddress(const char* path, struct sockaddr_un* addr, socklen_t* length) {
	if(!path || !addr || !length) {
		errno = EINVAL;
		return false;
	}

	if(strlen(path) >= sizeof(addr->sun_path)) {
		errno = ENAMETOOLONG;
		return false;
	}

	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, path);

	*length = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + strlen(addr->sun_path) + 1);

	return true;
}

int CreateIpcListener(const char* path) {
	struct sockaddr_un addr;
	socklen_t addrLength;

	if(!MakeIpcAddress(path, &addr, &addrLength)) {
		return -1;
	}

	int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);

	if(fd < 0) {
		return -1;
	}

	if(SetNonBlockingIpc(fd) < 0) {
		int error = errno;

		close(fd);
		errno = error;

		return -1;
	}

	unlink(path);

	if(bind(fd, (struct sockaddr*)&addr, addrLength) < 0) {
		int error = errno;

		close(fd);
		errno = error;

		return -1;
	}

	if(chmod(path, 0600) < 0) {
		int error = errno;

		close(fd);
		unlink(path);
		errno = error;

		return -1;
	}

	if(listen(fd, 16) < 0) {
		int error = errno;

		close(fd);
		unlink(path);
		errno = error;

		return -1;
	}

	return fd;
}

int IpcConnect(const char* path) {
	struct sockaddr_un addr;
	socklen_t addrLength;

	if(!MakeIpcAddress(path, &addr, &addrLength)) {
		return -1;
	}

	int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);

	if(fd < 0) {
		return -1;
	}

	if(connect(fd, (struct sockaddr*)&addr, addrLength) < 0) {
		int error = errno;

		close(fd);
		errno = error;

		return -1;
	}

	return fd;
}

int SetNonBlockingIpc(int fd) {
	int flags = fcntl(fd, F_GETFL);

	if(flags < 0) {
		return -1;
	}

	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		return -1;
	}

	int descriptorFlags = fcntl(fd, F_GETFD);

	if(descriptorFlags < 0) {
		return -1;
	}

	if(fcntl(fd, F_SETFD, descriptorFlags | FD_CLOEXEC) < 0) {
		return -1;
	}

	return 0;
}

int SetIpcTimeout(int fd, int milliseconds) {
	struct timeval timeout = {
		.tv_sec = milliseconds / 1000,
		.tv_usec = (milliseconds % 1000) * 1000
	};

	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		return -1;
	}

	if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		return -1;
	}

	return 0;
}

ssize_t SendIpcText(int fd, const char* text) {
	if(!text) {
		errno = EINVAL;
		return -1;
	}

	size_t length = strlen(text);
	ssize_t sent = send(fd, text, length, MSG_NOSIGNAL);

	if(sent == (ssize_t)length) {
		return sent;
	}

	if(sent >= 0) {
		errno = EIO;
	}

	return -1;
}

ssize_t ReceiveIpcText(int fd, char* buffer, size_t size) {
	if(!buffer || size == 0) {
		errno = EINVAL;
		return -1;
	}

	ssize_t received = recv(fd, buffer, size - 1, 0);

	if(received >= 0) {
		buffer[received] = '\0';
	}

	return received;
}
