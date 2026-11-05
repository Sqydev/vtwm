#ifndef VTWM_UTILS_ERRORS_H
#define VTWM_UTILS_ERRORS_H

#include <stdbool.h>
#include <stdio.h>

void* IFNULLPANIC(void* value, int exitCode, const char* msg, ...);
bool IFFALSEPANIC(bool value, int exitCode, const char* msg, ...);
void IFNULLPANICALONE(void* value, int exitCode, const char* msg, ...);
void IFFALSEPANICALONE(bool value, int exitCode, const char* msg, ...);
const void* IFNULLPANICCONST(const void* value, int exitCode, const char* msg, ...);
void IFNULLPANICALONECONST(const void* value, int exitCode, const char* msg, ...);

void InitPanic(void);
void vaPanic(int exitCode, const char* msg, va_list args);
void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));


#endif
