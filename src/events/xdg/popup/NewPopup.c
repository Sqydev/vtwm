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

#include "../../../utils/utils.h"

#include <stdlib.h>

#include <wlr/util/log.h>

typedef struct VtwmPopup {
	struct wlr_xdg_popup* xdg_popup;
	struct wl_listener commit;
	struct wl_listener destroy;
} VtwmPopup;

static void CommitPopup(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmPopup* popup = wl_container_of(listener, popup, commit);

	if(popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}
}

static void DestroyPopup(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmPopup* popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->commit.link);
	wl_list_remove(&popup->destroy.link);
	free(popup);
}

void NewPopup(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_xdg_popup* xdg_popup = data;

	VtwmPopup* popup = IFNULLPANIC(calloc(1, sizeof(*popup)), 1, "Failed to allocate popup\n");
	popup->xdg_popup = xdg_popup;

	struct wlr_xdg_surface* parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
	if(parent != NULL && parent->data != NULL) {
		struct wlr_scene_tree* popup_tree = wlr_scene_xdg_surface_create(parent->data, xdg_popup->base);
		if(popup_tree) {
			xdg_popup->base->data = popup_tree;
		}
		else {
			wlr_log(WLR_ERROR, "Failed to create scene tree for xdg popup");
		}
	}
	else {
		wlr_log(WLR_ERROR, "Xdg popup has no valid scene parent");
	}

	popup->commit.notify = CommitPopup;
	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

	popup->destroy.notify = DestroyPopup;
	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}
