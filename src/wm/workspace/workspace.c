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

#include "./workspace.h"

#include "../../coredata.h"
#include "../../utils/utils.h"
#include "../wm.h"

#include <stdlib.h>
#include <wlr/util/log.h>

void InitWorkspace(Workspace** workspace, VtwmOutput* output) {
	Workspace* ws = IFNULLPANIC(calloc(1, sizeof(*ws)), EXIT_FAILURE, "Failed to allocate workspace\n");

	for(int i = 0; i < LAYER_COUNT; i++) {
		ws->layers[i].windows = NULL;
		ws->layers[i].windowsCount = 0;
	}

	ws->currentLayer = LAYER_TILING;

	ws->output = output;

	*workspace = ws;
}

void NewWorkspace(WorkspaceDir* wdir, VtwmOutput* output) {
	size_t index = wdir->workspacesCount;

	wdir->workspaces = IFNULLPANIC(realloc(wdir->workspaces, (index + 1) * sizeof(*wdir->workspaces)), EXIT_FAILURE, "Failed to allocate workspace\n");

	InitWorkspace(&wdir->workspaces[index], output);

	wdir->workspacesCount++;
	wdir->workspaceIdx = index;
}

void CloseWorkspace(Workspace* workspace) {
	if(!workspace) {
		return;
	}

	for(int i = 0; i < LAYER_COUNT; i++) {
		for(size_t j = 0; j < workspace->layers[i].windowsCount; j++) {
			free(workspace->layers[i].windows[j]);
		}

		free(workspace->layers[i].windows);
		workspace->layers[i].windows = NULL;
		workspace->layers[i].windowsCount = 0;
	}

	free(workspace);
}

void SwitchToWorkspace(VtwmOutput* output, size_t workspace, size_t wdir, LayerName layer) {
	wlr_log(WLR_DEBUG, "output=%p, workspace=%p, wdir=%zu, wdirsCount=%zu, layer=%d, LAYER_COUNT=%d", (void*)output, output ? (void*)output->workspace : NULL, wdir, WM.wdirsCount, layer, LAYER_COUNT);
	if(!output || !output->workspace || wdir >= WM.wdirsCount || layer >= LAYER_COUNT) { return; }

	WorkspaceDir* dir = WM.wdirs[wdir];
	if(!dir) { return; }

	wlr_log(WLR_DEBUG, "Switching to workspace %zu, wdir %zu, layer %d", workspace, wdir, layer);

	while(dir->workspacesCount <= workspace) {
		wlr_log(WLR_DEBUG, "Creating workspace %zu", dir->workspacesCount);
		NewWorkspace(dir, NULL);
	}

	wlr_log(WLR_DEBUG, "Workspace pointer: %p", (void*)dir->workspaces[workspace]);

	Workspace* newWorkspace = dir->workspaces[workspace];

	if(!newWorkspace || (output->workspace == newWorkspace && output->workspace->currentLayer == layer)) { return; }

	WorkspaceLayer* currentWorkspacer = &output->workspace->layers[output->workspace->currentLayer];

	wlr_log(WLR_DEBUG, "OLD workspace: %p, layer: %d, windows: %zu", (void*)output->workspace, output->workspace->currentLayer, currentWorkspacer->windowsCount);

	for(size_t i = 0; i < currentWorkspacer->windowsCount; i++) {
		VtwmView* view = currentWorkspacer->windows[i]->view;

		wlr_log(WLR_DEBUG, "HIDE: view=%p mapped=%d visible=%d tree=%p", (void*)view, view->mapped, view->visible, (void*)view->scene_tree);

		view->visible = false;
		if(view->mapped) { wlr_scene_node_set_enabled(&view->scene_tree->node, false); }
	}

	output->workspace->output = NULL;
	output->workspace = newWorkspace;

	output->workspace->output = output;
	output->workspace->currentLayer = layer;

	WorkspaceLayer* newWorkspacer = &newWorkspace->layers[layer];

	for(size_t i = 0; i < newWorkspacer->windowsCount; i++) {
		VtwmView* view = newWorkspacer->windows[i]->view;

		view->visible = true;
		if(view->mapped) { wlr_scene_node_set_enabled(&view->scene_tree->node, true); }
	}

	VtwmSeat* seat = SeatGetActive();
	VtwmView* focus = NULL;

	for(size_t i = newWorkspacer->windowsCount; i > 0; i--) {
		VtwmView* view = newWorkspacer->windows[i - 1]->view;

		if(view && view->mapped) {
			focus = view;
			break;
		}
	}

	if(seat) {
		SeatFocusView(seat, focus);
	}

	wlr_log(WLR_DEBUG, "NEW workspace: %p, layer: %d, windows: %zu", (void*)newWorkspace, layer, newWorkspacer->windowsCount);
}
