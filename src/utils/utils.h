#ifndef VTWM_UTILS_H
#define VTWM_UTILS_H

#include <stddef.h>

void Close(int exitCode);
int Finish(void);

void FullWrite(int fd, const char* buffer, size_t size);

void InitPanic(void);
void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));

char* GetRandomFileFromDir(const char* dirPath);

char* JoinStrings(const char* a, const char* b);

#endif
