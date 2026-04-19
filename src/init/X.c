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

#include "../cleanup/cleanup.h"

#include "../coredata.h"
#include "../logging/logging.h"

#include <X11/Xlib.h>
#include <X11/Xproto.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int X11ErrorHandler(Display *display, XErrorEvent *event) {
    if(event->error_code == BadWindow || (event->request_code == X_SetInputFocus && event->error_code == BadMatch) || (event->request_code == X_ConfigureWindow && event->error_code == BadMatch)) {
        return 0;
    }

    char error_text[1024];
    XGetErrorText(display, event->error_code, error_text, sizeof(error_text));
    
    fprintf(stderr, "\n=== [VTWM ERROR] ===\n");
    fprintf(stderr, "X11 ERROR: %s\n", error_text);
    fprintf(stderr, "Request Code: %d\n", event->request_code);
    fprintf(stderr, "Error Code: %d\n", event->error_code);
    fprintf(stderr, "Resource ID: %lu\n", event->resourceid);
    fprintf(stderr, "====================\n\n");

    TraceLog("=== [VTWM ERROR] ===");
    TraceLog("X11 ERROR: %s", error_text);
    TraceLog("Request Code: %d", event->request_code);
    TraceLog("Error Code: %d", event->error_code);
    TraceLog("Resource ID: %lu", event->resourceid);
    TraceLog("====================");

	CleanUp();
	exit(EXIT_FAILURE);
}

void InitX11(void) {
	XSetErrorHandler(X11ErrorHandler);

	DATA.Rooty.Display = XOpenDisplay(NULL);

	if(!DATA.Rooty.Display) {
		printf("XOpenDisplay failed\n");
		CleanUp();
		exit(EXIT_FAILURE);
	}

	DATA.Rooty.Root = DefaultRootWindow(DATA.Rooty.Display);

	XSelectInput(DATA.Rooty.Display, DATA.Rooty.Root, SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | KeyPressMask);
}
