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

#include "./config.h"

#include "../coredata.h"
#include "../defines.h"
#include "../utils/utils.h"

#include <errno.h>
#include <string.h>

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

static void OpenConfigPath(const char* path) {
	FILE* file = IFNULLPANIC(fopen(path, "r"), EXIT_FAILURE, "Failed to open config in %s: %s\n", path, strerror(errno));

	char* pathCopy = strdup(path);
	if(!pathCopy) {
		fclose(file);
		Panic(EXIT_FAILURE, "Failed to open config in %s: %s\n", path, strerror(errno));
	}

	free(CONFIG.configPath);

	CONFIG.configPath = pathCopy;
	CONFIG.configFile = file;
}

void InitConfig(const char* overridePath) {
	CONFIG.configFile = NULL;
	CONFIG.configPath = NULL;
	CONFIG.configChainged = false;

	if(overridePath && *overridePath) {
		char* path = ExpandPath(overridePath);
		if(!path) {
			Panic(1, "Connot expand path: %s", overridePath);
		}
		OpenConfigPath(path);
		free(path);
		return;
	}

	const char* envPath = getenv("VTWM_CONFIG_PATH");
	if(envPath && *envPath) {
		char* path = ExpandPath(envPath);
		if(!path) {
			Panic(1, "Cannot expand path: %s", envPath);
		}
		OpenConfigPath(path);
		free(path);
		return;
	}

	char* userDir = GetUserConfigDir();
	char* userPath = userDir ? JoinStrings(userDir, "/" CONFIG_FILE_NAME) : NULL;

	if(userPath && PathExists(userPath)) {
		OpenConfigPath(userPath);
		free(userDir);
		free(userPath);
		return;
	}

	const char* defaultPath = VTWM_DEFAULT_CONFIG_PATH;

	if(!PathExists(defaultPath)) {
		Panic(1,
			"No config found.\n"
			"Tried:\n"
			"  %s\n"
			"  %s\n"
			"Create one or pass --config <path>.",
			userPath ? userPath : "(no user config dir)",
			defaultPath);
	}

	if(!userDir || !userPath) {
		OpenConfigPath(defaultPath);
		free(userDir);
		free(userPath);
		return;
	}

	if(!MakeDirP(userDir) || !CopyFile(defaultPath, userPath)) {
		Panic(1, "Cannot create %s: %s", userPath, strerror(errno));
	}

	OpenConfigPath(userPath);

	free(userDir);
	free(userPath);
}

bool ReloadConfig(void) {
	if(!CONFIG.configPath) { return false; }

	FILE* file = fopen(CONFIG.configPath, "r");
	if(!file) { return false; }

	if(CONFIG.configFile) { fclose(CONFIG.configFile); }
	CONFIG.configFile = file;

	return true;
}

void CloseConfig(void) {
	if(CONFIG.configFile) {
		fclose(CONFIG.configFile);
		CONFIG.configFile = NULL;
	}

	free(CONFIG.configPath);
	CONFIG.configPath = NULL;
}
