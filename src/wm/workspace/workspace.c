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

static void WorkspaceDropTTYSpawn(Workspace* workspace) {
	TtySpawn* ttySpawn = NULL;
	TtySpawn* ttySpawnTmp = NULL;
	wl_list_for_each_safe(ttySpawn, ttySpawnTmp, &WM.ttySpawns, link) {
		if(ttySpawn->workspace != workspace) {
			continue;
		}

		wl_list_remove(&ttySpawn->link);
		free(ttySpawn);
	}

	workspace->ttySpawnPending = false;
}

void CloseWorkspace(Workspace* workspace) {
	if(!workspace) {
		return;
	}

	WorkspaceDropTTYSpawn(workspace);
	workspace->ttyTerminal = NULL;

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

bool WorkspaceGetLocation(Workspace* workspace, size_t* wdirIdx, size_t* workspaceIdx) {
	if(!workspace) {
		return false;
	}

	for(size_t i = 0; i < WM.wdirsCount; i++) {
		WorkspaceDir* dir = WM.wdirs[i];
		if(!dir) {
			continue;
		}

		for(size_t j = 0; j < dir->workspacesCount; j++) {
			if(dir->workspaces[j] != workspace) {
				continue;
			}

			if(wdirIdx) { *wdirIdx = i; }
			if(workspaceIdx) { *workspaceIdx = j; }

			return true;
		}
	}

	return false;
}

void WorkspaceApplyViewVisibility(Workspace* workspace, struct VtwmView* view) {
	if(!workspace || !view) {
		return;
	}

	bool visible = workspace->output != NULL &&
		view->window != NULL &&
		view->window->workspace == workspace &&
		view->window->layer <= workspace->currentLayer &&
		view->mapped;

	view->visible = visible;

	if(view->scene_tree) {
		wlr_scene_node_set_enabled(&view->scene_tree->node, visible);
	}
}

void WorkspaceApplyLayerVisibility(Workspace* workspace) {
	if(!workspace) {
		return;
	}

	for(size_t layer = 0; layer < LAYER_COUNT; layer++) {
		WorkspaceLayer* workspaceLayer = &workspace->layers[layer];

		for(size_t i = 0; i < workspaceLayer->windowsCount; i++) {
			Window* window = workspaceLayer->windows[i];

			if(window && window->view) {
				WorkspaceApplyViewVisibility(workspace, window->view);
			}
		}
	}
}

static VtwmView* WorkspaceTopView(Workspace* workspace, LayerName layer) {
	if(!workspace || layer >= LAYER_COUNT) {
		return NULL;
	}

	WorkspaceLayer* workspaceLayer = &workspace->layers[layer];

	for(size_t i = workspaceLayer->windowsCount; i > 0; i--) {
		VtwmView* view = workspaceLayer->windows[i - 1]->view;

		if(view && view->mapped) {
			return view;
		}
	}

	return NULL;
}

static void WorkspaceFocusLayer(Workspace* workspace, LayerName layer) {
	VtwmSeat* seat = SeatGetActive();

	if(seat) {
		if(seat->wlr_seat) {
			wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
		}

		SeatFocusView(seat, WorkspaceTopView(workspace, layer));
	}
}

void WorkspaceEnsureTTY(Workspace* workspace, bool allowFallback) {
	if(!workspace || workspace->ttyTerminal || COMP.shuttingDown) {
		return;
	}

	if(workspace->ttySpawnPending) {
		if(allowFallback) {
			TtySpawn* pending = NULL;
			wl_list_for_each(pending, &WM.ttySpawns, link) {
				if(pending->workspace == workspace) {
					pending->allowFallback = true;
					break;
				}
			}
		}

		return;
	}

	if(!CONFIG.terminalCommandSliced || !CONFIG.terminalCommandSliced[0]) {
		wlr_log(WLR_ERROR, "Cannot create the TTY terminal: the terminal command is empty\n");
		return;
	}

	pid_t pid = Spawn(CONFIG.terminalCommandSliced);

	if(pid <= 0) {
		wlr_log(WLR_ERROR, "Failed to spawn the TTY terminal\n");
		return;
	}

	TtySpawn* ttySpawn = IFNULLPANIC(calloc(1, sizeof(*ttySpawn)), EXIT_FAILURE, "Failed to allocate TTY spawn\n");
	ttySpawn->workspace = workspace;
	ttySpawn->pid = pid;
	ttySpawn->allowFallback = allowFallback;

	wl_list_insert(&WM.ttySpawns, &ttySpawn->link);
	workspace->ttySpawnPending = true;

	wlr_log(WLR_DEBUG, "Spawned TTY terminal on workspace %p with pid %d", (void*)workspace, (int)pid);
}

Workspace* WorkspaceClaimTTY(pid_t pid) {
	TtySpawn* match = NULL;

	if(pid > 0) {
		TtySpawn* ttySpawn = NULL;
		wl_list_for_each(ttySpawn, &WM.ttySpawns, link) {
			if(ttySpawn->pid == pid) {
				match = ttySpawn;
				break;
			}
		}
	}

	if(!match) {
		TtySpawn* ttySpawn = NULL;
		wl_list_for_each(ttySpawn, &WM.ttySpawns, link) {
			if(ttySpawn->allowFallback) {
				match = ttySpawn;
				wlr_log(WLR_DEBUG, "Falling back to a pending TTY spawn\n");
				break;
			}
		}
	}

	if(!match) {
		return NULL;
	}

	Workspace* workspace = match->workspace;

	if(workspace) {
		workspace->ttySpawnPending = false;
	}

	wl_list_remove(&match->link);
	free(match);

	return workspace;
}

void WorkspaceTTYClaimed(Workspace* workspace, struct VtwmView* view) {
	if(!workspace || !view) {
		return;
	}

	workspace->ttyTerminal = view;
	workspace->ttySpawnPending = false;
}

void WorkspaceTTYClosed(Workspace* workspace, struct VtwmView* view) {
	if(!workspace || workspace->ttyTerminal != view) {
		return;
	}

	workspace->ttyTerminal = NULL;

	if(!COMP.shuttingDown) {
		WorkspaceEnsureTTY(workspace, workspace->output != NULL);
	}
}

bool WorkspaceIsTTYView(const struct VtwmView* view) {
	return view &&
		view->window &&
		view->window->workspace &&
		view->window->workspace->ttyTerminal == view;
}

void WorkspacePrewarmNext(Workspace* workspace) {
	size_t wdirIdx = 0;
	size_t workspaceIdx = 0;

	if(!WorkspaceGetLocation(workspace, &wdirIdx, &workspaceIdx) || wdirIdx >= WM.wdirsCount) {
		return;
	}

	WorkspaceDir* dir = WM.wdirs[wdirIdx];
	if(!dir) {
		return;
	}

	size_t nextWorkspace = workspaceIdx + 1;

	while(dir->workspacesCount <= nextWorkspace) {
		NewWorkspace(dir, NULL);
	}

	if(dir->workspaces[nextWorkspace]) {
		WorkspaceEnsureTTY(dir->workspaces[nextWorkspace], false);
	}
}

void SwitchToLayer(VtwmOutput* output, LayerName layer) {
	if(!output || !output->workspace || layer >= LAYER_COUNT) {
		return;
	}

	Workspace* workspace = output->workspace;

	if(workspace->currentLayer != layer) {
		workspace->currentLayer = layer;
		WorkspaceApplyLayerVisibility(workspace);
	}

	WorkspaceEnsureTTY(workspace, true);
	WorkspaceFocusLayer(workspace, layer);
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

	Workspace* newWorkspace = dir->workspaces[workspace];

	if(!newWorkspace) { return; }

	if(output->workspace == newWorkspace) {
		SwitchToLayer(output, layer);
		return;
	}

	Workspace* oldWorkspace = output->workspace;
	oldWorkspace->output = NULL;
	WorkspaceApplyLayerVisibility(oldWorkspace);

	output->workspace = newWorkspace;
	newWorkspace->output = output;
	newWorkspace->currentLayer = layer;

	WorkspaceApplyLayerVisibility(newWorkspace);
	WorkspaceEnsureTTY(newWorkspace, true);
	WorkspaceFocusLayer(newWorkspace, layer);
	WorkspacePrewarmNext(newWorkspace);

	wlr_log(WLR_DEBUG, "NEW workspace: %p, layer: %d", (void*)newWorkspace, layer);
}
