#include "./socket.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

int CreateIpcListener(const char* socketPath) {
	struct sockaddr_un socketAddress;
	socklen_t socketAddressLength;
	if(!MakeIpcAddress(socketPath, &socketAddress, &socketAddressLength)) {
		return -1;
	}

	// Local socket &&  massage oriented comms(send(X); send(Y) == "X", "Y"), and close on exec() so new procs don't break anything
	int socketFd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if(socketFd < 0) {
		return -1;
	}

	if(SetNonBlockingIpc(socketFd) < 0) {
		int savedErrno = errno;

		close(socketFd);
		errno = savedErrno;

		return -1;
	}

	unlink(socketPath);
	if(bind(socketFd, (struct sockaddr*)&socketAddress, socketAddressLength) < 0) {
		int savedErrno = errno;

		close(socketFd);
		errno = savedErrno;

		return -1;
	}

	if(chmod(socketPath, 0600) < 0) {
		int savedErrno = errno;

		close(socketFd);
		unlink(socketPath);
		errno = savedErrno;

		return -1;
	}

	// 16 max requests queued
	if(listen(socketFd, 16) < 0) {
		int savedErrno = errno;

		close(socketFd);
		unlink(socketPath);
		errno = savedErrno;

		return -1;
	}

	return socketFd;
}

int IpcConnect(const char* socketPath) {
	struct sockaddr_un socketAddress;
	socklen_t socketAddressLength;

	if(!MakeIpcAddress(socketPath, &socketAddress, &socketAddressLength)) {
		return -1;
	}

	int socketFd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);

	if(socketFd < 0) {
		return -1;
	}

	if(connect(socketFd, (struct sockaddr*)&socketAddress, socketAddressLength) < 0) {
		int savedErrno = errno;

		close(socketFd);
		errno = savedErrno;

		return -1;
	}

	return socketFd;
}
