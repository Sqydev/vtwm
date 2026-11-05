#ifndef VTWM_UTILS_SYSTEM_H
#define VTWM_UTILS_SYSTEM_H

#include <stdbool.h>
#include <stddef.h>

bool MakeDirP(const char* path);
bool MakeDir(const char* path);

bool CopyFile(const char* src, const char* dst);

void FullWrite(int fd, const char* buffer, size_t size);

char* GetRandomFile(const char* dirPath);

void Spawn(char** arguments);
void SpawnCommand(const char* command);
void SpawnCommandRange(const char* start, size_t length);

#endif
