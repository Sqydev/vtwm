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

#include "./management.h"

#include "../coredata.h"

#include <X11/X.h>
#include <stdlib.h>

void InitManagement(void) {
	DATA.Management.workspacesCount = 10;

	DATA.Management.Termode.windows = malloc(DATA.Management.minWorkspaces * sizeof(Window));
	DATA.Management.Termode.currentWorkspace = malloc(DATA.Monitors.Count * sizeof(int));

	for(int i = 0; i < DATA.Monitors.Count; i++) {
		DATA.Management.Termode.windows[i] = None;
	}
	for(int i = 0; i < DATA.Monitors.Count; i++) {
		DATA.Management.Termode.currentWorkspace[i] = i;
	}
}

// NOTE: MUAHAHAHAHAH, BIG ASS ARRAY IF STATMENT >:)
void SwitchToWorkspace(int workspace) {
	if(workspace == 0) { workspace++; }
	workspace--;

	if(DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]] != None) {
		XUnmapWindow(DATA.Rooty.Display, DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]]);
	}

	if(workspace >= DATA.Management.workspacesCount) {
		DATA.Management.Termode.windows = realloc(DATA.Management.Termode.windows, workspace * sizeof(Window));
		for(int i = DATA.Management.workspacesCount; i <= workspace; i++) {
		    DATA.Management.Termode.windows[i] = None;
		}
		DATA.Management.workspacesCount = workspace + 1;
	}
	DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent] = workspace;

	if(DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]] != None) {
		XMapWindow(DATA.Rooty.Display, DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]]);
	}
}

void CleanManagement(void) {
	if(DATA.Management.Termode.windows) { free(DATA.Management.Termode.windows); }
	if(DATA.Management.Termode.currentWorkspace) { free(DATA.Management.Termode.currentWorkspace); }
}
