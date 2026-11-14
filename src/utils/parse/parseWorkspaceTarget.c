#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

bool ParseWorkspaceTarget(const char* action, const char* open, int rangeValue, int* workspace) {
	const char* close = strrchr(action, ')');

	if(!open || !close || close <= open) { return false; }

	const char* cursor = open + 1;

	while(cursor < close && isspace((unsigned char)*cursor)) {
		cursor++;
	}

	if(strncmp(cursor, "argv[1]", 7) == 0) {
		cursor += 7;

		while(cursor < close && isspace((unsigned char)*cursor)) {
			cursor++;
		}

		int offset = 0;

		if(cursor < close && (*cursor == '+' || *cursor == '-')) {
			char* end = NULL;
			long value = strtol(cursor + 1, &end, 10);

			if(end == cursor + 1) { return false; }

			offset = *cursor == '-' ? -(int)value : (int)value;
			cursor = end;
		}

		while(cursor < close && isspace((unsigned char)*cursor)) {
			cursor++;
		}

		if(cursor != close) { return false; }

		*workspace = rangeValue + offset;

		return *workspace >= 0;
	}

	char* end = NULL;
	long value = strtol(cursor, &end, 10);

	if(end == cursor) { return false; }

	while(end < close && isspace((unsigned char)*end)) {
		end++;
	}

	if(end != close) { return false; }
	if(value < 0 || value > INT_MAX) { return false; }

	*workspace = (int)value;

	return true;
}
