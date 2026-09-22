#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void FullWrite(int fd, const char* buffer, size_t size) {
	size_t written = 0;

	while(written < size) {
		ssize_t n = write(fd, buffer + written, size - written);

		if(n > 0) {
			written += (size_t)n;
		}
		else if(n == -1) {
			if(errno == EINTR) {
				continue;
			}
			break;
		}
		else {
			break;
		}
	}
}
