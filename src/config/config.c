/* 
* Copyright (c) 2025-present W.K. ( _Sqyd_ / Sqydev )
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

#include "./defaults/defaults.h"
#include "./parser/parser.h"
#include "./sections/sections.h"

#include "../coredata.h"
#include "../defines.h"
#include "../utils/utils.h"

#include <errno.h>
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

static bool LoadConfigFile(void) {
	FILE* configFile = fopen(CONFIG.configPath, "r");
	if(!configFile) {
		fprintf(
			stderr,
			"Config warning: failed to open %s: %s\n",
			CONFIG.configPath,
			strerror(errno)
		);
		return false;
	}

	int fd = fileno(configFile);

	struct stat st;
	if(fstat(fd, &st) != 0 || st.st_size <= 0) {
		fprintf(stderr, "Config warning: config file is empty or unreadable: %s\n", CONFIG.configPath);
		fclose(configFile);
		return false;
	}

	char* configStr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(configStr == MAP_FAILED) {
		fprintf(
			stderr,
			"Config warning: failed to map %s: %s\n",
			CONFIG.configPath,
			strerror(errno)
		);
		fclose(configFile);
		return false;
	}

	ParseConfig(configStr, (size_t)st.st_size);

	munmap(configStr, (size_t)st.st_size);
	fclose(configFile);

	return true;
}

static bool TryConfigPath(const char* path, bool warnIfMissing) {
	char* expanded = ExpandPath(path);
	if(!expanded) {
		fprintf(stderr, "Config warning: cannot expand path: %s\n", path);
		return false;
	}

	if(!PathExists(expanded)) {
		if(warnIfMissing) {
			fprintf(stderr, "Config warning: config not found: %s\n", expanded);
		}

		free(expanded);
		return false;
	}

	free(CONFIG.configPath);
	CONFIG.configPath = expanded;

	if(LoadConfigFile()) { return true; }

	free(CONFIG.configPath);
	CONFIG.configPath = NULL;

	return false;
}

void LoadConfig(void) {
	char* explicitPath = CONFIG.configPath;
	CONFIG.configPath = NULL;

	if(explicitPath) {
		bool loaded = TryConfigPath(explicitPath, true);
		free(explicitPath);

		if(loaded) { return; }
	}

	const char* envPath = getenv("VTWM_CONFIG_PATH");
	if(envPath && *envPath && TryConfigPath(envPath, true)) { return; }

	char* userDir = GetUserConfigDir();
	char* userPath = userDir ? JoinStrings(userDir, "/" CONFIG_FILE_NAME) : NULL;
	bool userConfigExists = userPath && PathExists(userPath);

	if(userConfigExists) {
		bool loaded = TryConfigPath(userPath, false);

		free(userDir);
		free(userPath);

		if(loaded) { return; }

		fprintf(stderr, "Config warning: cannot load user config, using built-in defaults\n");
		ParseConfig(NULL, 0);
		return;
	}

	const char* defaultPath = VTWM_DEFAULT_CONFIG_PATH;
	bool loaded = false;

	if(PathExists(defaultPath)) {
		if(userDir && userPath && MakeDirP(userDir) && CopyFile(defaultPath, userPath)) {
			loaded = TryConfigPath(userPath, false);
		}
		else if(userDir && userPath) {
			fprintf(
				stderr,
				"Config warning: cannot create %s, loading %s\n",
				userPath,
				defaultPath
			);
		}

		if(!loaded) {
			loaded = TryConfigPath(defaultPath, false);
		}
	}

	free(userDir);
	free(userPath);

	if(loaded) { return; }

	fprintf(stderr, "Config warning: no config file found, using built-in defaults\n");
	ParseConfig(NULL, 0);
}

static void ParseCategoryBody(ConfigParser* parser, Token name, uint32_t layers) {
	if(TokenEquals(name, "Terminal")) {
		ParseTerminalBlock(parser);
	}
	else if(TokenEquals(name, "Looks")) {
		ParseLooksBlock(parser, layers);
	}
	else if(TokenEquals(name, "Cursor")) {
		ParseCursorBlock(parser);
	}
	else if(TokenEquals(name, "Input")) {
		ParseInputBlock(parser);
	}
	else if(TokenEquals(name, "StartUp")) {
		ParseStartUpBlock(parser);
	}
	else if(TokenEquals(name, "Keybinds")) {
		ParseKeybindsBlock(parser);
	}
	else {
		ConfigWarning(name.line, "Unknown category, ignoring");
		SkipBlockBody(parser, name.line);
	}
}

void ParseConfig(const char* data, size_t size) {
	SetConfigDefaults();

	ConfigParser parser;
	ConfigParserInit(&parser, data, size);

	bool startupSeen = false;
	bool keybindsSeen = false;

	while(parser.token.data) {
		if(ParserAccept(&parser, ";")) { continue; }

		if(TokenEquals(parser.token, "}")) {
			ConfigWarning(parser.token.line, "Unexpected '}'");
			ParserAdvance(&parser);
			continue;
		}

		Token name = parser.token;
		size_t entryLine = name.line;
		ParserAdvance(&parser);

		uint32_t layers = CONFIG_LAYER_ALL;

		if(ParserAccept(&parser, "->")) {
			layers = ParseLayerScope(&parser);
		}

		if(!ParserExpect(&parser, "{")) {
			SkipUnknownEntry(&parser, entryLine);
			continue;
		}

		if(TokenEquals(name, "StartUp") && !startupSeen) {
			ClearStartUp();
			startupSeen = true;
		}
		else if(TokenEquals(name, "Keybinds") && !keybindsSeen) {
			ClearKeybinds();
			keybindsSeen = true;
		}

		ParseCategoryBody(&parser, name, layers);

		ParserAccept(&parser, ";");
	}

	TerminalCommandSet();
}

void UnloadConfig(void) {
	ClearConfigValues();

	if(CONFIG.configPath) {
		free(CONFIG.configPath);
		CONFIG.configPath = NULL;
	}
}
