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
*	you must not claim that you wrote the original software. An acknowledgment in 
*	product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*	or files containing significant portions of this code, must be distributed under 
*	these same license terms. Such modified source code must be made publicly available 
*	to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*	used as a component of proprietary and closed-source products. In such cases, 
*	the surrounding proprietary application code does not need to be disclosed, 
*	provided that the original or modified source code of THIS software remains 
*	available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*	and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*	must not be removed. You may add your own credits to forks, provided the original 
*	authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*	source or binary distribution.
*/

#include "./config.h"

#include "../coredata.h"
#include "../defines.h"
#include "../utils/utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CONFIG_FILE_NAME "config.vtwm"

static char* GetUserConfigDir(void) {
	const char* xdg = getenv("XDG_CONFIG_HOME");
	if(xdg && xdg[0] == '/') {
		return JoinStrings(xdg, "/vtwm");
	}

	const char* home = getenv("HOME");
	if(!home) { return NULL; }

	return JoinStrings(home, "/.config/vtwm");
}

static void SetConfigPath(const char* path) {
	char* pathCopy = strdup(path);
	if(!pathCopy) {
		Panic(EXIT_FAILURE, "Failed to set config path %s: %s\n", path, strerror(errno));
	}

	free(CONFIG.configPath);

	CONFIG.configPath = pathCopy;
}

void LoadConfig(void) {
	if(CONFIG.configPath) {
		char* path = ExpandPath(CONFIG.configPath);
		if(!path) {
			Panic(1, "Cannot expand path: %s\n", CONFIG.configPath);
		}
		free(CONFIG.configPath);
		CONFIG.configPath = path;

		if(PathExists(CONFIG.configPath)) { goto loadConfig; }

		free(CONFIG.configPath);
		CONFIG.configPath = NULL;
	}

	const char* envPath = getenv("VTWM_CONFIG_PATH");
	if(envPath && *envPath) {
		char* path = ExpandPath(envPath);
		if(!path) {
			Panic(1, "Cannot expand path: %s\n", envPath);
		}
		if(PathExists(path)) {
			SetConfigPath(path);
			free(path);
			goto loadConfig;
		}
		free(path);
	}

	char* userDir = GetUserConfigDir();
	char* userPath = userDir ? JoinStrings(userDir, "/" CONFIG_FILE_NAME) : NULL;

	if(userPath && PathExists(userPath)) {
		SetConfigPath(userPath);
		free(userDir);
		free(userPath);
		goto loadConfig;
	}

	const char* defaultPath = VTWM_DEFAULT_CONFIG_PATH;

	if(!PathExists(defaultPath)) {
		Panic(1,
			"No config found.\n"
			"Tried:\n"
			"  %s\n"
			"  %s\n"
			"Create one or pass --config <path>.\n",
			userPath ? userPath : "(no user config dir)",
			defaultPath);
	}

	if(!userDir || !userPath) {
		SetConfigPath(defaultPath);
		free(userDir);
		free(userPath);
		goto loadConfig;
	}

	if(!MakeDirP(userDir) || !CopyFile(defaultPath, userPath)) {
		Panic(1, "Cannot create %s: %s\n", userPath, strerror(errno));
	}

	SetConfigPath(userPath);

	free(userDir);
	free(userPath);

loadConfig: ;

	FILE* configFile = IFNULLPANIC(fopen(CONFIG.configPath, "r"), 1, "Failed to open config file at: %s\n", CONFIG.configPath);
	int fd = fileno(configFile);

	struct stat st;
	fstat(fd, &st);

	char* configStr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(configStr == MAP_FAILED) {
		fclose(configFile);
		Panic(1, "Failed to mmap config file to string\n");
	}

	ParseConfig(configStr, st.st_size);

	if(configStr) { munmap(configStr, st.st_size); }
	if(configFile) { fclose(configFile); }
}

void ParseConfig(const char* data, size_t size) {
	for(size_t idx = 0; idx < size;) {
		while(idx < size && isspace(data[idx])) { idx++; }

		if(idx >= size) { break; }

		size_t start;
		size_t end;

		if(data[idx] == '"') {
			start = ++idx;

			while(idx < size && data[idx] != '"') { idx++; }

			end = idx;

			if(idx < size) { idx++; }
		}
		else {
			start = idx;

			while(idx < size && !isspace(data[idx])) { idx++; }

			end = idx;
		}

		size_t tokenLength = end - start;

		// Debug
		printf("%.*s\n", (int)tokenLength, data + start);
	}
}
