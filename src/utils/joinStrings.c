#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* JoinStrings(const char* a, const char* b) {
	size_t len = strlen(a) + strlen(b) + 1;
	char* result = malloc(len);

	if(!result) { return NULL; }

	snprintf(result, len, "%s%s", a, b);
	return result;
}
