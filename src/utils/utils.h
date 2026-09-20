#ifndef UTILS_VTWM_H
#define UTILS_VTWM_H

void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));

char* GetRandomFileFromDir(const char* dirPath);

char* JoinStrings(const char* a, const char* b);

#endif
