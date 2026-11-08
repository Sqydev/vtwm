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
