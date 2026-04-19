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

#include "../coredata.h"
#include "../cleanup/cleanup.h"
#include "../config/config.h"
#include "../logging/logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>

void InitConfig(void) {
    SetUpDefaultConfig();

    const char* home = getenv("HOME");
    if(!home) {
        fprintf(stderr, "HOME not set\n");
        CleanUp();
        exit(EXIT_FAILURE);
    }

    const char* config_path_fmt = "%s/.config/vtwm/vtwm.conf";
    const char* config_dir_fmt = "%s/.config/vtwm/";

    size_t path_len = snprintf(NULL, 0, config_path_fmt, home) + 1;
    size_t dir_len = snprintf(NULL, 0, config_dir_fmt, home) + 1;

    DATA.Config.path = malloc(path_len);
    DATA.Config.dir = malloc(dir_len);

    if(!DATA.Config.path || !DATA.Config.dir) {
		TraceLog("Malloc failed for config path or config dir. Panicing");
        CleanUp();
        exit(EXIT_FAILURE);
    }

    snprintf(DATA.Config.path, path_len, config_path_fmt, home);
    snprintf(DATA.Config.dir, dir_len, config_dir_fmt, home);

	TraceLog("Trying to load the config");
    if(!LoadConfig()) {
		TraceLog("Couldn't load the config. Generating fresh one");
    	GenerateConfig();
    }
}

void CleanUpConfig(void) {
	if(DATA.Config.path) {
		free(DATA.Config.path);
	}
	if(DATA.Config.dir) {
		free(DATA.Config.dir);
	}
	if(DATA.Config.termCommand) {
		free(DATA.Config.termCommand);
	}
}
