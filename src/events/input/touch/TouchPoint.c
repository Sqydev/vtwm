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

#include <stdlib.h>

#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_touch.h>

// The scene node (and with it the surface) went away, so there is no touch point left to follow.
// The client already got the surface destroy event.
static void TouchPointNodeDestroy(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmTouchPoint* point = wl_container_of(listener, point, scene_node_destroy);

	point->scene_node = NULL;
	wl_list_remove(&point->scene_node_destroy.link);
	wl_list_remove(&point->link);
	free(point);
}

VtwmTouchDevice* TouchDeviceFromTouch(VtwmSeat* seat, struct wlr_touch* touch) {
	if(!seat || !touch) {
		return NULL;
	}

	VtwmTouchDevice* device = NULL;
	wl_list_for_each(device, &seat->touch_devices, link) {
		if(device->wlr_touch == touch) {
			return device;
		}
	}

	return NULL;
}

VtwmTouchPoint* TouchPointFind(VtwmSeat* seat, int32_t touch_id) {
	if(!seat) {
		return NULL;
	}

	VtwmTouchPoint* point = NULL;
	wl_list_for_each(point, &seat->touch_points, link) {
		if(point->touch_id == touch_id) {
			return point;
		}
	}

	return NULL;
}

VtwmTouchPoint* TouchPointAdd(VtwmSeat* seat, VtwmTouchDevice* device, int32_t touch_id, struct wlr_scene_node* scene_node, double clip_x, double clip_y, uint32_t time_msec) {
	if(!seat || !scene_node) {
		return NULL;
	}

	VtwmTouchPoint* point = calloc(1, sizeof(*point));
	if(!point) {
		return NULL;
	}

	point->seat = seat;
	point->device = device;
	point->scene_node = scene_node;
	point->touch_id = touch_id;
	point->last_time_msec = time_msec;
	point->clip_x = clip_x;
	point->clip_y = clip_y;

	point->scene_node_destroy.notify = TouchPointNodeDestroy;
	wl_signal_add(&scene_node->events.destroy, &point->scene_node_destroy);

	wl_list_insert(&seat->touch_points, &point->link);

	return point;
}

void TouchPointRemove(VtwmTouchPoint* point) {
	if(!point) {
		return;
	}

	if(point->scene_node) {
		wl_list_remove(&point->scene_node_destroy.link);
		point->scene_node = NULL;
	}

	wl_list_remove(&point->link);
	free(point);
}

void TouchPointRemoveById(VtwmSeat* seat, int32_t touch_id) {
	TouchPointRemove(TouchPointFind(seat, touch_id));
}

void TouchPointsDestroy(VtwmSeat* seat) {
	if(!seat) {
		return;
	}

	VtwmTouchPoint* point = NULL;
	VtwmTouchPoint* tmp = NULL;
	wl_list_for_each_safe(point, tmp, &seat->touch_points, link) {
		TouchPointRemove(point);
	}
}
