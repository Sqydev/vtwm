#ifndef VTWM_UTILS_H
#define VTWM_UTILS_H

void InitPanic(void);
void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));

char* GetRandomFileFromDir(const char* dirPath);

char* JoinStrings(const char* a, const char* b);

#endif
