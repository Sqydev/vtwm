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

#ifndef VTWM_INPUT_INPUT_H
#define VTWM_INPUT_INPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <libinput.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/interfaces/wlr_touch.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>
#include <wlr/types/wlr_xcursor_manager.h>

#define INPUT_DEVICE_DIR "/dev/input"
// Major on the devices to take as input device(on linux the default is 13)(I ain't explaing it here but majors are cool. Check them out :))
#define INPUT_DEVICE_MAJOR 13

struct VtwmSeat;
struct VtwmView;
struct wlr_scene_node;
struct wlr_scene_surface;

typedef struct InputDevice {
	struct wl_list link;

	char* path;

	struct libinput_device* libinput_device;

	bool removed;

	dev_t rdev;

	struct wlr_keyboard keyboard;
	struct wlr_pointer pointer;
	struct wlr_touch touch;

	bool has_keyboard;
	bool has_pointer;
	bool has_touch;
} InputDevice;

typedef struct InputHardware {
	struct libinput* context;

	struct wl_event_source* libinput_source;
	struct wl_event_source* inotify_source;
	struct wl_event_source* reconcile_source;
	int inotify_fd;

	bool own_context;

	struct wl_list devices;
} InputHardware;

typedef struct InputManager {
	InputHardware Hardware;

	struct wl_list seats;
	struct VtwmSeat* active_seat;

	struct wlr_xcursor_manager* cursor_manager;
	struct wlr_cursor* cursor;

	size_t seat_counter;
} InputManager;

void InitInput(void);
void DestroyInput(void);
void SyncInput(void);
void ResyncInputSchedule(void);

#endif
