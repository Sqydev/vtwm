#ifndef VTWM_UTILS_STRINGS_H
#define VTWM_UTILS_STRINGS_H

#include <stddef.h>

typedef struct {
	const char* data;
	size_t length;
	size_t line;
} Token;

char* JoinStrings(const char* a, const char* b);
char** SplitString(const char* string);
void FreeStringArray(char** array);

Token NextToken(const char* data, size_t size, size_t* idx, size_t* line);
int TokenEquals(Token token, const char* str);

#endif
