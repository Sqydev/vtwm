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
