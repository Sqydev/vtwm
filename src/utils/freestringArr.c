#include "./utils.h"

#include <stdlib.h>

void FreeStringArray(char** array) {
	if(!array) { return; }

	for(size_t i = 0; array[i] != NULL; i++) {
		free(array[i]);
	}

	free(array);
}
