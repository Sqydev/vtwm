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
#include "../../utils/utils.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <sys/types.h>

void DoMapRequest(void) {
	if(!DATA.Management.tilingOn) {
	    Window window = DATA.events.xmaprequest.window;

		int assignedMonitor = -1;
		pid_t windowPid = GetWindowPid(window);
		bool IsItBuildInTerm = false; // Or is it :) ?

		if(windowPid > 0) {
			for(int i = DATA.Monitors.Count - 1; i >= 0; i--) {
				if(DATA.Monitors.mtermsPids[i] == windowPid) {
					assignedMonitor = i;
					DATA.Monitors.mtermsPids[i] = -1;
					IsItBuildInTerm = true;
					break;
				}
			}
		}

		if(!IsItBuildInTerm) {
			DATA.Management.Termode.windows[DATA.Management.Termode.currentWorkspace[DATA.Monitors.Currrent]] = window;
		}

	    XineramaScreenInfo monitor = (assignedMonitor >= 0) ? DATA.Monitors.Thing[assignedMonitor] : DATA.Monitors.Thing[DATA.Monitors.Currrent];

	    XWindowChanges changes;

	    changes.x = monitor.x_org;
	    changes.y = monitor.y_org;
	    changes.width = monitor.width;
	    changes.height = monitor.height;

	    XConfigureWindow(DATA.Rooty.Display, window, CWX | CWY | CWWidth | CWHeight, &changes);
	
	    XMapWindow(DATA.Rooty.Display, window);

		if(assignedMonitor <= 0) {
	    	XSetInputFocus(DATA.Rooty.Display, window, RevertToPointerRoot, CurrentTime);
		}

	    XSelectInput(DATA.Rooty.Display, window, EnterWindowMask);
	}
}
