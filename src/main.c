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

#include "./coredata.h"

#include <X11/Xlib.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

CoreData DATA;

void spawn_xterm() {
    pid_t pid = fork();

    if (pid == 0) {
        char *args[] = {"xterm", NULL};
        execvp("xterm", args);

        perror("execvp failed");
        _exit(1);
    }
}

// NOTE: I'm learning so I'll comment the shit out of this
int main() {
	// NOTE: Get the connention with X server
	DATA.Display = XOpenDisplay(NULL);

	if(!DATA.Display) {
		printf("ERROR: There's arleady a WM RUNNING?!\n");
		return EXIT_FAILURE;
	}

	// NOTE: Like do root thing. Like it's root of all windows
	DATA.Root = DefaultRootWindow(DATA.Display);

	// NOTE: What events do I wanna get and take control over(I'll split it up)
	XSelectInput(
		DATA.Display, // NOTE: To which Display
		DATA.Root, // NOTE: To which Root
		SubstructureRedirectMask // NOTE: ALL OF THEM
		| // NOTE: And
		SubstructureNotifyMask // NOTE: Notyfy about things like: New window, type shit
	);

	spawn_xterm();

	while(1) {
		// NOTE: Blocking(so yk no 100% cpu usadge and works only if there's work to do)
		// Wait for next event from X server and save it to DATA.events
		XNextEvent(DATA.Display, &DATA.events);

		// NOTE: Accualy do somethig with events(split into types)
		switch(DATA.events.type) {
			// NOTE: Some app want's to get it's window shown :)
			case MapRequest: {
				// NOTE: SHOW IT
				XMapWindow(DATA.Display, DATA.events.xmaprequest.window);
				
				break;
			}

			case ConfigureRequest: {
				// NOTE: What app wants
			    XConfigureRequestEvent* AppWant = &DATA.events.xconfigurerequest;
	
				// NOTE: What we'll chainge becouse we are cherry pickng
			    XWindowChanges changes;
			    changes.x = AppWant->x;
			    changes.y = AppWant->y;
			    changes.width = AppWant->width;
			    changes.height = AppWant->height;
			    changes.border_width = AppWant->border_width;
			    changes.sibling = AppWant->above;
			    changes.stack_mode = AppWant->detail;
			
				// NOTE: Accualy configure it
			    XConfigureWindow(DATA.Display, AppWant->window, AppWant->value_mask, &changes);
			    break;
			}
		}
	}

	return EXIT_SUCCESS;
}
