#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool GetRuntimeDir(char* path, size_t size) {
	if(!path || size == 0) {
		errno = EINVAL;
		return false;
	}

	const char* runtime = getenv("XDG_RUNTIME_DIR");

	if(!runtime || runtime[0] == '\0') {
		errno = ENOENT;
		return false;
	}

	int written = snprintf(path, size, "%s", runtime);

	if(written <= 0 || (size_t)written >= size) {
		errno = ENAMETOOLONG;
		return false;
	}

	return true;
}
