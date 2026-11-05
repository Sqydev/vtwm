#include "./paths.h"

#include <sys/stat.h>

bool PathExists(const char* path) {
	struct stat st;
	return path && stat(path, &st) == 0;
}
