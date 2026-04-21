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

#include "../utils/utils.h"
#include "../logging/logging.h"

#include <stdlib.h>
#include <string.h>

void InitTerminals(void) {
	TraceLog("Starting terminals");

    DATA.Terminals.pids = malloc(DATA.Monitors.Count * sizeof(pid_t));

    for(int i = 0; i < DATA.Monitors.Count; i++) {
        DATA.Terminals.pids[i] = -1;
    }

	size_t tookcnt = 0;
	char* ptr = DATA.Config.termCommand;
	while(*ptr != '\0') {
    	if(*ptr == ' ') { tookcnt++; }
    	ptr++;
    	while(*ptr == ' ') { ptr++; }
	}

	char** spawnArr = malloc((tookcnt + 2) * sizeof(char*));
	char* token = strtok(DATA.Config.termCommand, " ");
	for(size_t i = 0; token != NULL; i++) {
    	spawnArr[i] = malloc((strlen(token) + 1) * sizeof(char));
    	strcpy(spawnArr[i], token);
    	token = strtok(NULL, " ");
	}
	spawnArr[tookcnt + 1] = NULL;

	TraceLog("Termianl command == %s", DATA.Config.termCommand);
	for(size_t i = 0; i <= tookcnt; i++) {
		TraceLog("Termianl command tokenised == %s", spawnArr[i]);
	}

    for(int i = 1; i < DATA.Monitors.Count; i++) {
        pid_t pid = Spawn(1, DATA.Config.termCommand);
        if(pid > 0) {
            DATA.Terminals.pids[i] = pid;
        }
    }
	
    pid_t pid = SpawnArr(spawnArr);
    if(pid > 0) {
        DATA.Terminals.pids[0] = pid;
    }

	for(size_t i = 0; i < tookcnt; i++) {
		free(spawnArr[i]);
	}
	free(spawnArr);
}
