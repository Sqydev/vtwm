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

#include "./wm.h"

#include "./wdir/wdir.h"
#include "./workspace/workspace.h"

#include "../coredata.h"
#include "../utils/errors/errors.h"

#include <stdlib.h>
#include <string.h>

void InitWM(void) {
	memset(&WM, 0, sizeof(WindowManager));
}

void FinishWM(void) {
	CloseWdirs();

	memset(&WM, 0, sizeof(WindowManager));
}

void AddWindow(Workspace* workspace, VtwmView* view, LayerName layer) {
	if(!workspace || !view || layer >= LAYER_COUNT) {
		return;
	}

	WorkspaceLayer* workspaceLayer = &workspace->layers[layer];

	size_t index = workspaceLayer->windowsCount;

	workspaceLayer->windows = IFNULLPANIC(realloc(workspaceLayer->windows, (index + 1) * sizeof(*workspaceLayer->windows)), EXIT_FAILURE, "Failed to allocate workspace windows\n");

	Window* window = IFNULLPANIC(calloc(1, sizeof(*window)), EXIT_FAILURE, "Failed to allocate window\n");

	window->view = view;
	window->workspace = workspace;
	window->layer = layer;
	window->index = index;

	WorkspaceGetLocation(workspace, &window->wdirIdx, &window->workspaceIdx);

	view->window = window;

	workspaceLayer->windows[index] = window;
	workspaceLayer->windowsCount++;
}

void RemoveWindow(VtwmView* view) {
	if(!view || !view->window) {
		return;
	}

	Window* window = view->window;
	view->window = NULL;

	Workspace* workspace = window->workspace;
	if(!workspace || window->layer >= LAYER_COUNT) {
		free(window);
		return;
	}

	WorkspaceLayer* workspaceLayer = &workspace->layers[window->layer];
	if(!workspaceLayer->windows) {
		free(window);
		return;
	}

	size_t index = window->index;
	if(index >= workspaceLayer->windowsCount || workspaceLayer->windows[index] != window) {
		index = workspaceLayer->windowsCount;

		for(size_t i = 0; i < workspaceLayer->windowsCount; i++) {
			if(workspaceLayer->windows[i] == window) {
				index = i;
				break;
			}
		}

		if(index == workspaceLayer->windowsCount) {
			free(window);
			return;
		}
	}

	for(size_t i = index + 1; i < workspaceLayer->windowsCount; i++) {
		workspaceLayer->windows[i - 1] = workspaceLayer->windows[i];
		workspaceLayer->windows[i - 1]->index = i - 1;
	}

	workspaceLayer->windowsCount--;

	if(workspaceLayer->windowsCount == 0) {
		free(workspaceLayer->windows);
		workspaceLayer->windows = NULL;
	}

	free(window);
}
