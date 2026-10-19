#ifndef VTWM_UTILS_H
#define VTWM_UTILS_H

#include <stddef.h>
#include <stdbool.h>

void* IFNULLPANIC(void* value, int exitCode, const char* msg);
bool IFFALSEPANIC(bool value, int exitCode, const char* msg);
void IFNULLPANICALONE(void* value, int exitCode, const char* msg);
void IFFALSEPANICALONE(bool value, int exitCode, const char* msg);
const void* IFNULLPANICCONST(const void* value, int exitCode, const char* msg);
void IFNULLPANICALONECONST(const void* value, int exitCode, const char* msg);

void Close(int exitCode);
int Finish(void);

void FullWrite(int fd, const char* buffer, size_t size);

void InitPanic(void);
void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));

char* GetRandomFileFromDir(const char* dirPath);

char* JoinStrings(const char* a, const char* b);

#endif
