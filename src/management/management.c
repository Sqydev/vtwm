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

#include "../utils/utils.h"
#include "../coredata.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>

void InitManagement(void) {
	DATA.Management.Termode.workspacesCount = DATA.Management.minWorkspaces;

	DATA.Management.Termode.windows = malloc(DATA.Management.minWorkspaces * sizeof(Window*));
	DATA.Management.Termode.windowsCount = malloc(DATA.Management.minWorkspaces * sizeof(int));
	DATA.Management.Termode.currentWorkspace = malloc(DATA.Monitors.Count * sizeof(int));

	for(int i = 0; i < DATA.Management.minWorkspaces; i++) {
		DATA.Management.Termode.windows[i] = None;
		DATA.Management.Termode.windowsCount[i] = 0;
	}
	for(int i = 0; i < DATA.Monitors.Count; i++) {
		DATA.Management.Termode.currentWorkspace[i] = i;
	}
}

void AddWindowToWorkspace(int workspace, Window window) {
	DATA.Management.Termode.windows[workspace] = realloc(DATA.Management.Termode.windows[workspace], (DATA.Management.Termode.windowsCount[workspace] + 1) * sizeof(Window));
	DATA.Management.Termode.windows[workspace][DATA.Management.Termode.windowsCount[workspace]] = window;
	DATA.Management.Termode.windowsCount[workspace]++;
}

void RemoveWindowFromWorkspace(int workspace, Window window) {
	for(int i = 0; i < DATA.Management.Termode.windowsCount[workspace]; i++) {
		if(DATA.Management.Termode.windows[workspace][i] == window) {
			for(int j = i; j < DATA.Management.Termode.windowsCount[workspace] - 1; j++) {
				DATA.Management.Termode.windows[workspace][j] = DATA.Management.Termode.windows[workspace][j + 1];
			}
			DATA.Management.Termode.windowsCount[workspace]--;
			return;
		}
	}
}

// NOTE: MUAHAHAHAHAH, BIG ASS ARRAY IF STATMENT >:)
void DoSwitchToWorkspaceStuff(int workspace) {
	if(workspace == 0) { workspace++; }
	workspace--;

	if(DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent] == workspace) { return; }

	for(int i = 0; i < DATA.Monitors.Count; i++) {
		if(DATA.Management.Termode.currentWorkspace[i] == workspace) {
			XWarpPointer(DATA.Rooty.Display, None, DefaultRootWindow(DATA.Rooty.Display), 0, 0, 0, 0, DATA.Monitors.Thing[i].x_org + (DATA.Monitors.Thing[i].width / 2), DATA.Monitors.Thing[i].y_org + (DATA.Monitors.Thing[i].height / 2));
			XSetInputFocus(DATA.Rooty.Display, GetWindowUnderCursor(), RevertToPointerRoot, CurrentTime);
		}
	}
 
	for(int i = 0; i < DATA.Management.Termode.windowsCount[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]]; i++) {
		Window w = DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]][i];
		if(w != None) {
			XUnmapWindow(DATA.Rooty.Display, w);
		}
	}
 
	if(workspace >= DATA.Management.Termode.workspacesCount) {
		DATA.Management.Termode.windows = realloc(DATA.Management.Termode.windows, (workspace + 1) * sizeof(Window*));
		DATA.Management.Termode.windowsCount = realloc(DATA.Management.Termode.windowsCount, (workspace + 1) * sizeof(int));
 
		for(int i = DATA.Management.Termode.workspacesCount; i < workspace + 1; i++) {
			DATA.Management.Termode.windows[i] = None;
			DATA.Management.Termode.windowsCount[i] = 0;
		}
		DATA.Management.Termode.workspacesCount = workspace + 1;

        if(DATA.Config.termCommandArr) {
			SpawnArr(DATA.Config.termCommandArr);
		} else {
			Spawn(1, DATA.Config.termCommand);
		}
	}
 
	DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent] = workspace;
 
	for(int i = 0; i < DATA.Management.Termode.windowsCount[workspace]; i++) {
		Window w = DATA.Management.Termode.windows[workspace][i];
		if(w != None) {
			XMapWindow(DATA.Rooty.Display, w);
		}
	}
}

void CleanManagement(void) {
	if(DATA.Management.Termode.windows) {
		for(int i = 0; i < DATA.Management.Termode.workspacesCount; i++) {
			free(DATA.Management.Termode.windows[i]);
		}
		free(DATA.Management.Termode.windows);
	}
	free(DATA.Management.Termode.windowsCount);
	free(DATA.Management.Termode.currentWorkspace);
}

