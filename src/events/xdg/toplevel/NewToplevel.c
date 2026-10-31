/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
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

#include "../xdg.h"

#include "../../../coredata.h"
#include "../../../utils/utils.h"

#include <stdlib.h>

#include <wlr/util/log.h>

void NewToplevel(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_xdg_toplevel* xdg_toplevel = data;

	VtwmView* view = IFNULLPANIC(calloc(1, sizeof(*view)), 1, "Failed to allocate view\n");

	view->xdg_toplevel = xdg_toplevel;
	view->scene_tree = IFNULLPANIC(wlr_scene_xdg_surface_create(&COMP.scene->tree, xdg_toplevel->base), 1, "Failed to create scene tree\n");

	view->scene_tree->node.data = view;
	xdg_toplevel->base->data = view->scene_tree;

	view->map.notify = MapToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &view->map);

	view->unmap.notify = UnmapToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &view->unmap);

	view->commit.notify = CommitToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &view->commit);

	view->destroy.notify = DestroyToplevel;
	wl_signal_add(&xdg_toplevel->events.destroy, &view->destroy);

	view->request_maximize.notify = RequestToplevelMaximize;
	wl_signal_add(&xdg_toplevel->events.request_maximize, &view->request_maximize);

	view->request_fullscreen.notify = RequestToplevelFullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &view->request_fullscreen);

	wlr_log(WLR_INFO, "New toplevel: %s (%s)", xdg_toplevel->title ? xdg_toplevel->title : "?", xdg_toplevel->app_id ? xdg_toplevel->app_id : "?");
}
