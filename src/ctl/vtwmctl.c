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

#include "./vtwmctl.h"

#include "../ipc/ipc.h"
#include "../defines.h"
#include "../utils/utils.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int RunCtl(int argc, char** argv) {
	if(argc != 3) {
		fprintf(stderr, "Usage: vtwm ctl <command>\n");
		return 1;
	}

	char ipcDir[PATH_MAX];

	if(!GetIpcRuntimeDir(ipcDir, sizeof(ipcDir))) {
		fprintf(stderr, "Cannot determine IPC directory: %s\n", strerror(errno));
		return 1;
	}

	DIR* dir = opendir(ipcDir);
	if(!dir) {
		if(errno == ENOENT) {
			fprintf(stderr, "No running vtwm instance found\n");
			return 1;
		}

		fprintf(stderr, "Cannot open IPC directory %s: %s\n", ipcDir, strerror(errno));
		return 1;
	}

	size_t found = 0;
	size_t handled = 0;
	size_t prefixLength = strlen(IPC_SOCKET_PREFIX);
	size_t suffixLength = strlen(IPC_SOCKET_SUFFIX);
	struct dirent* entry;

	while((entry = readdir(dir))) {
		const char* name = entry->d_name;
		size_t nameLength = strlen(name);

		if(nameLength <= prefixLength + suffixLength) {
			continue;
		}

		if(strncmp(name, IPC_SOCKET_PREFIX, prefixLength) != 0) {
			continue;
		}

		if(strcmp(name + nameLength - suffixLength, IPC_SOCKET_SUFFIX) != 0) {
			continue;
		}

		char path[PATH_MAX];
		int written = snprintf(path, sizeof(path), "%s/%s", ipcDir, name);

		if(written <= 0 || (size_t)written >= sizeof(path)) {
			continue;
		}

		found++;

		int fd = IpcConnect(path);

		if(fd < 0) {
			if(errno == ECONNREFUSED) {
				unlink(path);
			}

			continue;
		}

		if(SetIpcTimeout(fd, IPC_TIMEOUT_MS) < 0) {
			fprintf(stderr, "%s: %s\n", path, strerror(errno));
			close(fd);
			continue;
		}

		if(SendIpcText(fd, argv[2]) < 0) {
			fprintf(stderr, "%s: %s\n", path, strerror(errno));
			close(fd);
			continue;
		}

		char response[IPC_RESPONSE_MAX];
		ssize_t received = ReceiveIpcText(fd, response, sizeof(response));

		close(fd);

		if(received <= 0) {
			fprintf(stderr, "%s: no response\n", path);
			continue;
		}

		response[strcspn(response, "\r\n")] = '\0';

		if(strcmp(response, "ok") == 0) {
			printf("%s: ok\n", name);
			handled++;
		}
		else {
			fprintf(stderr, "%s: %s\n", path, response[0] ? response : "error");
		}
	}

	closedir(dir);

	if(found == 0) {
		fprintf(stderr, "No running vtwm instance found\n");
		return 1;
	}

	if(handled == 0) {
		fprintf(stderr, "No vtwm instance handled the command\n");
		return 1;
	}

	return 0;
}
