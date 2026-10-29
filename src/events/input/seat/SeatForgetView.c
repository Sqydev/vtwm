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

#include <wlr/types/wlr_seat.h>

// The view is gone, so no seat can point at it anymore
void SeatForgetView(struct VtwmView* view) {
	VtwmSeat* seat = NULL;
	wl_list_for_each(seat, &INPUT.seats, link) {
		if(seat->focused_view != view) {
			continue;
		}

		seat->focused_view = NULL;

		// The keyboard should not go dead when the focused window closes
		VtwmView* next = NULL;
		VtwmView* candidate = NULL;
		wl_list_for_each(candidate, &COMP.views, link) {
			if(candidate != view) {
				next = candidate;
				break;
			}
		}

		if(next) {
			SeatFocusView(seat, next);
		}
		else if(seat->wlr_seat) {
			wlr_seat_keyboard_notify_clear_focus(seat->wlr_seat);
		}
	}
}
