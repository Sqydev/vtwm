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
#include "../../../compositor/compositor.h"
#include "../../../utils/utils.h"

#include <wlr/types/wlr_cursor.h>

// The cursor is shared by every seat, so its listeners live in EVENTS.Input
// instead of a single seat. The callbacks pick the active seat when they run.
void InitCursor(void) {
	INPUT.cursor = IFNULLPANIC(wlr_cursor_create(), 1, "Failed to create cursor\n");
	wlr_cursor_attach_output_layout(INPUT.cursor, COMP.output_layout);

	// All seats share one xcursor manager
	if(INPUT.cursor_manager) {
		wlr_cursor_set_xcursor(INPUT.cursor, INPUT.cursor_manager, "default");
	}

	EVENTS.Input.cursor_motion.notify = PointerMotion;
	wl_signal_add(&INPUT.cursor->events.motion, &EVENTS.Input.cursor_motion);

	EVENTS.Input.cursor_motion_absolute.notify = PointerMotionAbsolute;
	wl_signal_add(&INPUT.cursor->events.motion_absolute, &EVENTS.Input.cursor_motion_absolute);

	EVENTS.Input.cursor_button.notify = PointerButton;
	wl_signal_add(&INPUT.cursor->events.button, &EVENTS.Input.cursor_button);

	EVENTS.Input.cursor_axis.notify = PointerAxis;
	wl_signal_add(&INPUT.cursor->events.axis, &EVENTS.Input.cursor_axis);

	EVENTS.Input.cursor_frame.notify = PointerFrame;
	wl_signal_add(&INPUT.cursor->events.frame, &EVENTS.Input.cursor_frame);

	// The touch events come from the cursor, because that's where the touches are translated into the scene coordinates
	EVENTS.Input.touch_down.notify = TouchDown;
	wl_signal_add(&INPUT.cursor->events.touch_down, &EVENTS.Input.touch_down);

	EVENTS.Input.touch_up.notify = TouchUp;
	wl_signal_add(&INPUT.cursor->events.touch_up, &EVENTS.Input.touch_up);

	EVENTS.Input.touch_motion.notify = TouchMotion;
	wl_signal_add(&INPUT.cursor->events.touch_motion, &EVENTS.Input.touch_motion);

	EVENTS.Input.touch_cancel.notify = CancelTouch;
	wl_signal_add(&INPUT.cursor->events.touch_cancel, &EVENTS.Input.touch_cancel);

	EVENTS.Input.touch_frame.notify = TouchFrame;
	wl_signal_add(&INPUT.cursor->events.touch_frame, &EVENTS.Input.touch_frame);
}
