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

#include "../input.h"

#include "../../xdg/xdg.h"

#include <wlr/types/wlr_scene.h>

// Finds the toplevel that's under the given coordinates, walk up the scene tree until vtwm finds the node that carries the view.
// Popups and subsurfaces belong to the toplevel they are attached to.
VtwmView* SeatViewAt(VtwmSeat* seat, double x, double y, struct wlr_scene_surface** out_surface, struct wlr_scene_node** out_node, double* sx, double* sy) {
	if(!seat) {
		return NULL;
	}

	struct wlr_scene_node* node = SeatBufferAt(x, y, sx, sy);
	if(!node) {
		return NULL;
	}

	struct wlr_scene_surface* scene_surface = wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node));
	if(!scene_surface) {
		return NULL;
	}

	if(out_surface) {
		*out_surface = scene_surface;
	}
	if(out_node) {
		*out_node = node;
	}

	// The view is stored on the scene tree of its toplevel.
	// Subsurfaces and popups have their own nodes without data, so keep walking up.
	struct wlr_scene_node* current = node;
	while(current && !current->data) {
		current = current->parent ? &current->parent->node : NULL;
	}

	return current ? current->data : NULL;
}
