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
#include "../../../utils/utils.h"

#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>

void NewTouchDevice(struct wlr_input_device* device) {
	if(!device) {
		return;
	}

	VtwmSeat* seat = SeatGetActive();
	if(!seat) {
		seat = AutoCreateSeat();
	}
	if(INPUT.cursor) {
		wlr_cursor_attach_input_device(INPUT.cursor, device);
	}

	// Remember the device, so the clients learn when it's gone
	VtwmTouchDevice* touch = IFNULLPANIC(calloc(1, sizeof(*touch)), 1, "Failed to allocate touch device\n");
	touch->seat = seat;
	touch->wlr_touch = wlr_touch_from_input_device(device);
	touch->destroy.notify = DestroyTouchDevice;
	wl_signal_add(&device->events.destroy, &touch->destroy);
	wl_list_insert(&seat->touch_devices, &touch->link);
	seat->touch_count++;

	// Now the clients can actually use the touch
	SeatUpdateCapabilities(seat);
}
