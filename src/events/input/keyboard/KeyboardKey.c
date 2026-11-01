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

#include <unistd.h>

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon-keysyms.h>

void KeyboardKey(struct wl_listener* listener, void* data) {
	VtwmSeat* seat = wl_container_of(listener, seat, keyboard_key);

	struct wlr_keyboard* keyboard = seat->keyboard;
	struct wlr_keyboard_key_event* event = data;

	// Xkb uses evdev keycode + 8
	uint32_t keycode = event->keycode + 8;

	const xkb_keysym_t* syms = NULL;
	int nsyms = 0;
	if(keyboard && keyboard->xkb_state) {
		nsyms = xkb_state_key_get_syms(keyboard->xkb_state, keycode, &syms);
	}
	uint32_t mods = keyboard ? wlr_keyboard_get_modifiers(keyboard) : 0;

	// The seat that gets the keys is the active one from now on
	if(event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		SeatMakeActive(seat);
	}

	// Only handle shortcuts on active seat
	if(seat == INPUT.active_seat && event->state == WL_KEYBOARD_KEY_STATE_PRESSED && nsyms > 0) {
		if((mods & WLR_MODIFIER_ALT) && syms[0] == XKB_KEY_Escape) {
			Close(0);
		}
		if((mods & WLR_MODIFIER_ALT) && syms[0] == XKB_KEY_Tab) {
			pid_t pid = fork();

			if(pid == 0) {
				execvp(CONFIG.terminalCommandSliced[0], CONFIG.terminalCommandSliced);
				_exit(1);
			}
		}
	}

	if(!seat->wlr_seat || !keyboard) {
		return;
	}

	wlr_seat_keyboard_notify_key(seat->wlr_seat, event->time_msec, event->keycode, event->state);
}
