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

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

// The finger moved, so the surface that got the touch on the down gets the new position.
void TouchMotion(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_touch_motion_event* event = data;
	VtwmSeat* seat = SeatFromTouch(event->touch);

	if(!seat || !INPUT.cursor || !seat->wlr_seat) {
		return;
	}

	// The touch coordinates are normalised (0..1), so vtwm translates them to the layout coordinates first
	double lx = 0.0;
	double ly = 0.0;
	wlr_cursor_absolute_to_layout_coords(INPUT.cursor, &event->touch->base, event->x, event->y, &lx, &ly);

	// A tracked finger stays on the surface it went down on.
	// Moving outside that surface is valid and clients use it for dragging, so it must not be dropped just because another surface is under the finger now.
	VtwmTouchPoint* point = TouchPointFind(seat, event->touch_id);
	if(point && point->scene_node) {
		int origin_x = 0;
		int origin_y = 0;
		if(!wlr_scene_node_coords(point->scene_node, &origin_x, &origin_y)) {
			return;
		}

		double sx = lx - origin_x + point->clip_x;
		double sy = ly - origin_y + point->clip_y;

		point->last_time_msec = event->time_msec;
		wlr_seat_touch_notify_motion(seat->wlr_seat, event->time_msec, event->touch_id, sx, sy);
		return;
	}

	// A touch that wasn't tracked (for example one that started before the compositor got it) still gets the old behaviour
	struct wlr_touch_point* seat_point = wlr_seat_touch_get_point(seat->wlr_seat, event->touch_id);
	if(!seat_point || !seat_point->surface) {
		return;
	}

	double sx = 0.0;
	double sy = 0.0;
	struct wlr_scene_surface* scene_surface = SeatSurfaceAt(seat, lx, ly, &sx, &sy);
	if(!scene_surface || scene_surface->surface != seat_point->surface) {
		return;
	}

	wlr_seat_touch_notify_motion(seat->wlr_seat, event->time_msec, event->touch_id, sx, sy);
}
