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

#include "../../../coredata.h"
#include "../../xdg/xdg.h"

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

// The finger went down, so the surface under it gets the focus
void TouchDown(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_touch_down_event* event = data;
	VtwmSeat* seat = SeatFromTouch(event->touch);

	if(!seat || !INPUT.cursor || !seat->wlr_seat) {
		return;
	}

	// The seat that gets the touch is the active one from now on
	SeatMakeActive(seat);

	// the touch coordinates are normalised (0..1), so vtwm translates them to the layout coordinates first
	double lx = 0.0;
	double ly = 0.0;
	wlr_cursor_absolute_to_layout_coords(INPUT.cursor, &event->touch->base, event->x, event->y, &lx, &ly);

	double sx = 0.0;
	double sy = 0.0;
	struct wlr_scene_surface* scene_surface = NULL;
	struct wlr_scene_node* scene_node = NULL;
	VtwmView* view = SeatViewAt(seat, lx, ly, &scene_surface, &scene_node, &sx, &sy);
	if(!scene_surface || !scene_node) {
		return;
	}

	// The finger focuses the window it landed on, like a click would
	if(view) {
		SeatFocusView(seat, view);
	}

	// Remember where this finger started, so its motion can stay on this surface even after the finger leaves it
	int origin_x = 0;
	int origin_y = 0;
	double clip_x = 0.0;
	double clip_y = 0.0;
	if(wlr_scene_node_coords(scene_node, &origin_x, &origin_y)) {
		clip_x = sx - (lx - origin_x);
		clip_y = sy - (ly - origin_y);
	}

	// A down for the same id means the old point is stale
	TouchPointRemoveById(seat, event->touch_id);
	VtwmTouchDevice* device = TouchDeviceFromTouch(seat, event->touch);
	VtwmTouchPoint* point = TouchPointAdd(seat, device, event->touch_id, scene_node, clip_x, clip_y, event->time_msec);

	wlr_seat_touch_notify_down(seat->wlr_seat, scene_surface->surface, event->time_msec, event->touch_id, sx, sy);

	// The client may reject the touch (for example when it doesn't accept touch input), so don't keep a point that the seat doesn't know about
	if(point && !wlr_seat_touch_get_point(seat->wlr_seat, event->touch_id)) {
		TouchPointRemove(point);
	}
}
