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

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <sys/time.h>

int SetNonBlockingIpc(int socketFd) {
	int fileStatusFlags = fcntl(socketFd, F_GETFL);

	if(fileStatusFlags < 0) {
		return -1;
	}

	if(fcntl(socketFd, F_SETFL, fileStatusFlags | O_NONBLOCK) < 0) {
		return -1;
	}

	int fileDescriptorFlags = fcntl(socketFd, F_GETFD);
	if(fileDescriptorFlags < 0) {
		return -1;
	}

	if(fcntl(socketFd, F_SETFD, fileDescriptorFlags | FD_CLOEXEC) < 0) {
		return -1;
	}

	return 0;
}

int SetIpcTimeout(int socketFd, int milliseconds) {
	struct timeval socketTimeout = {
		.tv_sec = milliseconds / 1000,
		.tv_usec = (milliseconds % 1000) * 1000
	};

	if(setsockopt(
		socketFd,
		SOL_SOCKET,
		SO_RCVTIMEO,
		&socketTimeout,
		sizeof(socketTimeout)
	) < 0) {
		return -1;
	}

	if(setsockopt(
		socketFd,
		SOL_SOCKET,
		SO_SNDTIMEO,
		&socketTimeout,
		sizeof(socketTimeout)
	) < 0) {
		return -1;
	}

	return 0;
}

ssize_t SendIpcText(int socketFd, const char* text) {
	if(!text) {
		errno = EINVAL;
		return -1;
	}

	size_t textLength = strlen(text);
	// For MSG_NOSIGNAL go to man. :)
	ssize_t sentLength = send(socketFd, text, textLength, MSG_NOSIGNAL);
	if(sentLength == (ssize_t)textLength) {
		return sentLength;
	}

	if(sentLength >= 0) {
		errno = EIO;
	}

	return -1;
}

ssize_t ReceiveIpcText(int socketFd, char* buffer, size_t bufferSize) {
	if(!buffer || bufferSize == 0) {
		errno = EINVAL;
		return -1;
	}

	ssize_t receivedLength = recv(socketFd, buffer, bufferSize - 1, 0);
	if(receivedLength >= 0) {
		buffer[receivedLength] = '\0';
	}

	return receivedLength;
}
