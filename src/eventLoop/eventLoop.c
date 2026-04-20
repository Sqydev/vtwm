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

#include "../coredata.h"
#include "../cleanup/cleanup.h"

#include <X11/Xlib.h>

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

void EventLoop(void) {
	while(1) {
		// NOTE: Blocking(so yk no 100% cpu usadge and works only if there's work to do)
		// Wait for next event from X server and save it to DATA.events
		XNextEvent(DATA.Rooty.Display, &DATA.events);

		// NOTE: Accualy do somethig with events(split into types)
		switch(DATA.events.type) {
			// NOTE: Some app want's to get it's window shown :)
			case MapRequest: {
			    Window window = DATA.events.xmaprequest.window;

			    Atom netWmPid = XInternAtom(DATA.Rooty.Display, "_NET_WM_PID", False);
			    Atom cardinal = XInternAtom(DATA.Rooty.Display, "CARDINAL", False);
			    Atom actualType;
			    int actualFormat;
			    unsigned long nItems, bytesAfter;
			    unsigned char* prop = NULL;
			    pid_t windowPid = -1;

			    if(XGetWindowProperty(DATA.Rooty.Display, window, netWmPid, 0, 1, False, cardinal, &actualType, &actualFormat, &nItems, &bytesAfter, &prop) == Success && prop) {
			        windowPid = (pid_t)(*(unsigned long*)prop);
			        XFree(prop);
			    }

			    int assignedMonitor = -1;
			    if(windowPid > 0) {
			        for(int i = 0; i < DATA.Monitors.Count; i++) {
			            if(DATA.Terminals.pids[i] == windowPid) {
			                assignedMonitor = i;
			                DATA.Terminals.pids[i] = -1;
			                break;
			            }
			        }
			    }

			    XineramaScreenInfo monitor = (assignedMonitor >= 0) ? DATA.Monitors.Thing[assignedMonitor] : DATA.Monitors.Thing[DATA.Monitors.Currrent];

			    XWindowChanges changes;
			    changes.x = monitor.x_org;
			    changes.y = monitor.y_org;
			    changes.width = monitor.width;
			    changes.height = monitor.height;

			    XConfigureWindow(DATA.Rooty.Display, window, CWX | CWY | CWWidth | CWHeight, &changes);

			    XMapWindow(DATA.Rooty.Display, window);

			    XSetInputFocus(DATA.Rooty.Display, window, RevertToPointerRoot, CurrentTime);

			    XSelectInput(DATA.Rooty.Display, window, EnterWindowMask);

			    break;
			}

			case KeyPress: {
			    KeySym sym = XLookupKeysym(&DATA.events.xkey, 0);

				if (sym == XK_m) {
       				if((DATA.events.xkey.state & Mod4Mask) && (DATA.events.xkey.state & Mod1Mask)) {
       					CleanUp();
						exit(EXIT_SUCCESS);
					}
   				}

   				break;
			}


			case ConfigureRequest: {
				// NOTE: What app wants
			    XConfigureRequestEvent* AppWant = &DATA.events.xconfigurerequest;
	
				// NOTE: What we'll chainge becouse we are cherry pickng
			    XWindowChanges changes;
			    changes.border_width = AppWant->border_width;
			    changes.sibling = AppWant->above;
			    changes.stack_mode = AppWant->detail;
			
				// NOTE: Accualy configure it
			    XConfigureWindow(DATA.Rooty.Display, AppWant->window, AppWant->value_mask, &changes);
			    break;
			}

			case EnterNotify: {
				Window window = DATA.events.xcrossing.subwindow;
    			if(window == None || window == DATA.Rooty.Root) { window = DATA.events.xcrossing.window; }

    			if(DATA.events.xcrossing.mode != NotifyNormal) { break; }

				XSetInputFocus( // NOTE: Set focus
				    DATA.Rooty.Display,
				    window, // NOTE: Which window
				    RevertToPointerRoot, // NOTE: When window desintegrates then focus goes to window bellow
				    CurrentTime // NOTE: Time cuz it's async
				);

				// NOTE: And not update monitor
				int x;
				int y;

    			Window root;
				Window child;

    			int win_x;
				int win_y;

    			unsigned int mask;

				// NOTE: Take cursor pos. I wont comment
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

				// NOTE: Do monitor loop and do box collision
				for(int i = 0; i < DATA.Monitors.Count; i++) {
			        XineramaScreenInfo m = DATA.Monitors.Thing[i];
			
			        if(x >= m.x_org && x < m.x_org + m.width && y >= m.y_org && y < m.y_org + m.height) {
						DATA.Monitors.Currrent = i;
			            break;
			        }
			    }

				break;
			}
		}
	}
}
