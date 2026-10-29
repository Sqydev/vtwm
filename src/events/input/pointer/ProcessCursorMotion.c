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

// The cursor moved, so vtwm looks what's under it and give the focus to it.
// The coordinates vtwm sends are the ones inside the surface, not in the scene
void ProcessCursorMotion(VtwmSeat* seat, uint32_t time) {
	if(!seat || !INPUT.cursor || !seat->wlr_seat) {
		return;
	}

	double sx = 0.0;
	double sy = 0.0;

	struct wlr_scene_surface* scene_surface = SeatSurfaceAt(seat, INPUT.cursor->x, INPUT.cursor->y, &sx, &sy);
	// Nothing under the cursor, so nobody has the focus now
	if(!scene_surface) {
		if(INPUT.cursor_manager) {
			wlr_cursor_set_xcursor(INPUT.cursor, INPUT.cursor_manager, "default");
		}
		wlr_seat_pointer_notify_clear_focus(seat->wlr_seat);
		return;
	}

	// The enter event also makes the surface the focused one
	wlr_seat_pointer_notify_enter(seat->wlr_seat, scene_surface->surface, sx, sy);
	wlr_seat_pointer_notify_motion(seat->wlr_seat, time, sx, sy);
}
