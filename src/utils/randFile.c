#include "../linclude/stb_image.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

char* GetRandomFileFromDir(const char* dirPath) {
	DIR* dir = opendir(dirPath);
	if(!dir) { return NULL; }

	struct dirent* entry;
	char** files = NULL;
	size_t count = 0;

	while((entry = readdir(dir))) {
		if(entry->d_name[0] == '.') { continue; }

		char** tmp = realloc(files, (count + 1) * sizeof(*files));
		if(!tmp) { break; }

		files = tmp;
		files[count++] = strdup(entry->d_name);
	}

	closedir(dir);

	if(count == 0) {
		free(files);
		return NULL;
	}

	size_t index = (size_t)rand() % count;

	char* result = malloc(strlen(dirPath) + 1 + strlen(files[index]) + 1);

	if(result) {
		sprintf(result, "%s/%s", dirPath, files[index]);
	}

	for(size_t i = 0; i < count; i++) { free(files[i]); }

	free(files);

	return result;
}
