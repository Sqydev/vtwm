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

#include "./headers/coredata.h"
#include "./headers/cleanup.h"
#include "./headers/utils.h"
#include "config/config.h"

#include <X11/Xlib.h>
#include <X11/Xproto.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void SpawnTerminals(void) {
	DATA.Init.leftTerms = DATA.Monitors.Count;

    for(int i = 0; i < DATA.Monitors.Count; i++) {
        Spawn(1, "alacritty");
    }
}

int WindowManagerErrorHandler(Display *display, XErrorEvent *event) {
    if(event->error_code == BadWindow || (event->request_code == X_SetInputFocus && event->error_code == BadMatch) || (event->request_code == X_ConfigureWindow && event->error_code == BadMatch)) {
        return 0;
    }

    char error_text[1024];
    XGetErrorText(display, event->error_code, error_text, sizeof(error_text));
    
    fprintf(stderr, "\n=== [WM ERROR] ===\n");
    fprintf(stderr, "X11 ERROR: %s\n", error_text);
    fprintf(stderr, "Request Code: %d\n", event->request_code);
    fprintf(stderr, "Error Code: %d\n", event->error_code);
    fprintf(stderr, "Resource ID: %lu\n", event->resourceid);
    fprintf(stderr, "==================\n\n");

	CleanUp();
	exit(EXIT_FAILURE);
}

void SetUpDefaultConfig(void) {
	// NOTE: Set scale
	// If there isn't config there set to fallback 96
	system("xrdb -merge <<< \"Xft.dpi: 96\"");
}

void Init(void) {
	SetUpDefaultConfig();

	const char *home = getenv("HOME");
	if(!home) {
		fprintf(stderr, "HOME not set\n");
		CleanUp();
		exit(EXIT_FAILURE);
	}

	if(asprintf(&DATA.Config.path, "%s/.config/vtwm/vtwm.conf", home) == -1) {
		fprintf(stderr, "asprintf failed\n");
		CleanUp();
		exit(EXIT_FAILURE);
	}
	if(asprintf(&DATA.Config.dir, "%s/.config/vtwm/", home) == -1) {
		fprintf(stderr, "asprintf failed\n");
		CleanUp();
		exit(EXIT_FAILURE);
	}

	if(!LoadConfig()) {
		GenerateConfig();
	}

	// NOTE: Get the connention with X server
	DATA.Rooty.Display = XOpenDisplay(NULL);

	if(!DATA.Rooty.Display) {
		printf("XOpenDisplay failed\n");
		CleanUp();
		exit(EXIT_FAILURE);
	}

	// NOTE: Like do root thing. Like it's root of all windows
	DATA.Rooty.Root = DefaultRootWindow(DATA.Rooty.Display);

	// NOTE: What events do I wanna get and take control over(I'll split it up)
	XSelectInput(
		DATA.Rooty.Display, // NOTE: To which Display
		DATA.Rooty.Root, // NOTE: To which Root
		SubstructureRedirectMask // NOTE: ALL OF THEM
		| // NOTE: And
		SubstructureNotifyMask // NOTE: Notyfy about things like: New window, type shit
		|
		EnterWindowMask // NOTE: Notyfy when mouse is on window
		|
		KeyPressMask
	);

	if(!XineramaIsActive(DATA.Rooty.Display)) {
	    fprintf(stderr, "Xinerama not active\n");
		CleanUp();
	    exit(EXIT_FAILURE);
	}

	DATA.Monitors.Thing = XineramaQueryScreens(DATA.Rooty.Display, &DATA.Monitors.Count);

	if(!DATA.Monitors.Thing || DATA.Monitors.Count <= 0) {
	    fprintf(stderr, "No screens found\n");
		CleanUp();
	    exit(EXIT_FAILURE);
	}

	DATA.Monitors.Currrent = 0;
	
	SpawnTerminals();

	XWarpPointer(DATA.Rooty.Display, None, DefaultRootWindow(DATA.Rooty.Display), 0, 0, 0, 0, DATA.Monitors.Thing[0].x_org + (DATA.Monitors.Thing[0].width / 2), DATA.Monitors.Thing[0].y_org + (DATA.Monitors.Thing[0].height / 2));

	// NOTE: Make errors don't crash by default(for example doing action with some window that doesn't exist just stops the action)
	XSetErrorHandler(WindowManagerErrorHandler);
}
