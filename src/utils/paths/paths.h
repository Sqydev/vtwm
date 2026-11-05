#ifndef VTWM_UTILS_PATHS_H
#define VTWM_UTILS_PATHS_H

#include <stdbool.h>

char* ExpandPath(const char* path);

char* GetHomePath(void);
char* GetXdgRuntimePath(void);

bool PathExists(const char* path);

#endif
