/*
* Copyright (c) 2025-present W.K. ( _Sqyd_ / Sqydev )
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

#include "../../coredata.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

void DoEnterNotify(void) {
	Window window = DATA.events.xcrossing.subwindow;
    if(window == None || window == DATA.Rooty.Root) { window = DATA.events.xcrossing.window; }

    if(DATA.events.xcrossing.mode != NotifyNormal) { return; }

	XSetInputFocus( // NOTE: Set focus
	    DATA.Rooty.Display,
	    window, // NOTE: Which window
	    RevertToPointerRoot, // NOTE: When window desintegrates then focus goes to window bellow
	    CurrentTime // NOTE: Time cuz it's async
	);

	int x;
	int y;

    Window root;
	Window child;

    int win_x;
	int win_y;

    unsigned int mask;

	XQueryPointer(
        DATA.Rooty.Display,
        DATA.Rooty.Root,
        &root,
        &child,
        &x,
        &y,
        &win_x,
        &win_y,
        &mask
    );

	for(int i = 0; i < DATA.Monitors.Count; i++) {
        XineramaScreenInfo m = DATA.Monitors.Thing[i];

        if(x >= m.x_org && x < m.x_org + m.width && y >= m.y_org && y < m.y_org + m.height) {
			DATA.Monitors.Currrent = i;
            break;
        }
    }
}
