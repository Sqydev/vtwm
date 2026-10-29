/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

#include "../linclude/stb_image.h"

#include <dirent.h>
#include <stdio.h>
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
		char* name = strdup(entry->d_name);
		if(!name) {
			break;
		}
		files[count++] = name;
	}

	closedir(dir);

	if(count == 0) {
		free(files);
		return NULL;
	}

	size_t index = (size_t)rand() % count;

	size_t length = strlen(dirPath) + 1 + strlen(files[index]) + 1;
	char* result = malloc(length);

	if(result) {
		int written = snprintf(result, length, "%s/%s", dirPath, files[index]);
		if(written < 0 || (size_t)written >= length) {
			free(result);
			result = NULL;
		}
	}

	for(size_t i = 0; i < count; i++) { free(files[i]); }

	free(files);

	return result;
}
