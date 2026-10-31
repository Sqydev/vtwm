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

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

void SetKeyboardSeat(VtwmSeat* seat, struct wlr_keyboard* keyboard) {
	if(!seat || !keyboard) {
		return;
	}

	if(seat->keyboard == keyboard) {
		wlr_seat_set_keyboard(seat->wlr_seat, keyboard);
		return;
	}

	if(seat->keyboard) {
		wl_list_remove(&seat->keyboard_key.link);
		wl_list_remove(&seat->keyboard_modifiers.link);
		wl_list_remove(&seat->keyboard_destroy.link);
		wlr_seat_set_keyboard(seat->wlr_seat, NULL);
	}

	seat->keyboard = keyboard;

	seat->keyboard_key.notify = KeyboardKey;
	wl_signal_add(&keyboard->events.key, &seat->keyboard_key);

	seat->keyboard_modifiers.notify = KeyboardModifiers;
	wl_signal_add(&keyboard->events.modifiers, &seat->keyboard_modifiers);

	seat->keyboard_destroy.notify = DestroyKeyboard;
	wl_signal_add(&keyboard->base.events.destroy, &seat->keyboard_destroy);

	wlr_seat_set_keyboard(seat->wlr_seat, keyboard);

	// Now the clients can use the keyboard
	SeatUpdateCapabilities(seat);

	// A window focused before this keyboard arrived has to get the enter
	SeatKeyboardEnterFocused(seat);
}
