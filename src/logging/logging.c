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
#include "../cleanup/cleanup.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void InitLogging(void) {
    char date[64];
    time_t now = time(NULL);
    struct tm* currTime = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", currTime);

    const char* home = getenv("HOME");

    size_t pathLen = snprintf(NULL, 0, "%s/.var/log/vtwm/%s.log", home, date) + 1;
    size_t dirLen = snprintf(NULL, 0, "%s/.var/log/vtwm", home) + 1;

    DATA.Logging.logPath = malloc(pathLen);
    DATA.Logging.logDir = malloc(dirLen);

    if(!DATA.Logging.logPath || !DATA.Logging.logDir) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    snprintf(DATA.Logging.logPath, pathLen, "%s/.var/log/vtwm/%s.log", home, date);
    snprintf(DATA.Logging.logDir,  dirLen,  "%s/.var/log/vtwm", home);

    if(mkdir_p(DATA.Logging.logDir, 0777) != 0) {
        fprintf(stderr, "mkdir_p for %s failed\n", DATA.Logging.logDir);
        CleanUp();
        exit(1);
    }

    DATA.Logging.logFile = fopen(DATA.Logging.logPath, "w");
    printf("\033[41m YOU YES YOU! \033[0m From now on all of the logs will land in: %s\n", DATA.Logging.logPath);
    printf("\033[41m YOU YES YOU! \033[0m From now on all of the logs will land in: %s\n", DATA.Logging.logPath);
    printf("\033[41m YOU YES YOU! \033[0m From now on all of the logs will land in: %s\n", DATA.Logging.logPath);
}

void TraceLog(const char* log, ...) {
	if(!DATA.Logging.logFile) { return; }

    va_list va;
    va_start(va, log);

    char date[64];
    time_t now = time(NULL);
    struct tm* currTime = localtime(&now);

    strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", currTime);
	fprintf(DATA.Logging.logFile, "[%s] ", date);
	vfprintf(DATA.Logging.logFile, log, va);
	fprintf(DATA.Logging.logFile, "\n");
	fflush(DATA.Logging.logFile);

	va_end(va);
}

void CleanUpLogging(void) {
	if(DATA.Logging.logFile) {
		fclose(DATA.Logging.logFile);
	}
	if(DATA.Logging.logDir) {
		free(DATA.Logging.logDir);
	}
	if(DATA.Logging.logPath) {
		free(DATA.Logging.logPath);
	}
}
