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

#ifndef VTWM_EVENTS_INPUT_INPUT_H
#define VTWM_EVENTS_INPUT_INPUT_H

#include <stdint.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

typedef struct VtwmTouchDevice {
	struct wl_list link;
	struct wl_listener destroy;
	struct VtwmSeat* seat;
	struct wlr_touch* wlr_touch;
} VtwmTouchDevice;

typedef struct VtwmTouchPoint {
	struct wl_list link;
	struct wl_listener scene_node_destroy;
	struct VtwmSeat* seat;
	struct VtwmTouchDevice* device;
	struct wlr_scene_node* scene_node;
	int32_t touch_id;
	uint32_t last_time_msec;
	double clip_x;
	double clip_y;
} VtwmTouchPoint;

typedef struct VtwmSeat {
	struct wl_list link;
	char* name;

	struct wlr_seat* wlr_seat;
	struct wlr_keyboard* keyboard;

	struct wl_listener keyboard_key;
	struct wl_listener keyboard_modifiers;
	struct wl_listener keyboard_destroy;

	struct wl_listener request_set_cursor;
	struct wl_listener pointer_focus_change;

	struct VtwmView* focused_view;

	struct wl_list touch_devices;
	struct wl_list touch_points;
	uint32_t touch_count;
} VtwmSeat;

// Global input events. They belong to the backend and the shared cursor, so they cannot live in a single seat.
typedef struct InputEvents {
	struct wl_listener new_input;

	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	struct wl_listener touch_down;
	struct wl_listener touch_up;
	struct wl_listener touch_motion;
	struct wl_listener touch_cancel;
	struct wl_listener touch_frame;
} InputEvents;

void NewInput(struct wl_listener* listener, void* data);

void NewInputDevice(struct wlr_input_device* device);
void NewKeyboardDevice(struct wlr_input_device* device);
void NewPointerDevice(struct wlr_input_device* device);
void NewTouchDevice(struct wlr_input_device* device);

void KeyboardKey(struct wl_listener* listener, void* data);
void KeyboardModifiers(struct wl_listener* listener, void* data);
void DestroyKeyboard(struct wl_listener* listener, void* data);
void SetKeymap(struct wlr_keyboard* keyboard);

void PointerMotion(struct wl_listener* listener, void* data);
void PointerMotionAbsolute(struct wl_listener* listener, void* data);
void PointerButton(struct wl_listener* listener, void* data);
void PointerAxis(struct wl_listener* listener, void* data);
void PointerFrame(struct wl_listener* listener, void* data);
void RequestSetCursor(struct wl_listener* listener, void* data);
void ChangePointerFocus(struct wl_listener* listener, void* data);
void ProcessCursorMotion(VtwmSeat* seat, uint32_t time);

void TouchDown(struct wl_listener* listener, void* data);
void TouchUp(struct wl_listener* listener, void* data);
void TouchMotion(struct wl_listener* listener, void* data);
void CancelTouch(struct wl_listener* listener, void* data);
void TouchFrame(struct wl_listener* listener, void* data);
void DestroyTouchDevice(struct wl_listener* listener, void* data);

VtwmSeat* SeatFromTouch(struct wlr_touch* touch);

VtwmTouchDevice* TouchDeviceFromTouch(VtwmSeat* seat, struct wlr_touch* touch);
VtwmTouchPoint* TouchPointFind(VtwmSeat* seat, int32_t touch_id);
VtwmTouchPoint* TouchPointAdd(VtwmSeat* seat, VtwmTouchDevice* device, int32_t touch_id, struct wlr_scene_node* scene_node, double clip_x, double clip_y, uint32_t time_msec);
void TouchPointRemove(VtwmTouchPoint* point);
void TouchPointRemoveById(VtwmSeat* seat, int32_t touch_id);
void TouchPointsDestroy(VtwmSeat* seat);

void SeatFocusView(VtwmSeat* seat, struct VtwmView* view);
void SeatKeyboardEnter(VtwmSeat* seat, struct VtwmView* view);
void SeatKeyboardEnterFocused(VtwmSeat* seat);
void SeatForgetView(struct VtwmView* view);

struct wlr_scene_node* SeatBufferAt(double x, double y, double* sx, double* sy);
struct wlr_scene_surface* SeatSurfaceAt(VtwmSeat* seat, double x, double y, double* sx, double* sy);

struct VtwmView* SeatViewAt(VtwmSeat* seat, double x, double y, struct wlr_scene_surface** out_surface, struct wlr_scene_node** out_node, double* sx, double* sy);

void InitCursor(void);
void DestroyCursor(void);

VtwmSeat* CreateSeat(const char* name);
VtwmSeat* AutoCreateSeat(void);
void DestroySeat(VtwmSeat* seat);
void SetKeyboardSeat(VtwmSeat* seat, struct wlr_keyboard* keyboard);
VtwmSeat* SeatGetActive(void);
void SeatMakeActive(VtwmSeat* seat);
VtwmSeat* SeatFindOrCreateForNewKeyboard(void);
void SeatUpdateCapabilities(VtwmSeat* seat);

void InitSeatPointer(VtwmSeat* seat);
void DestroySeatPointer(VtwmSeat* seat);
void InitSeatTouch(VtwmSeat* seat);
void DestroySeatTouch(VtwmSeat* seat);
void DestroySeatKeyboard(VtwmSeat* seat);

#endif
