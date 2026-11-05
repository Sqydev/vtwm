#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

char* GetSelfPath(size_t* size) {
	pid_t pid = getpid();
	char link[PATH_MAX];
	sprintf(link, "/proc/%d/exe", pid);

	char* path = malloc(PATH_MAX * sizeof(char));

	ssize_t n = readlink(link, path, PATH_MAX);
	if(n == -1) {
		return NULL;
	}

	path[n + 1] = '\0';
	if(size) { *size = (n + 1) * sizeof(char); }

	return path;
}
