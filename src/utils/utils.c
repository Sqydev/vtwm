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
#include "../logging/logging.h"
#include "../coredata.h"

#include <X11/X.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

void CloseFocused(void) {
	Window focused;
	int revert_to;
	XGetInputFocus(DATA.Rooty.Display, &focused, &revert_to);

    if(focused != None && focused != PointerRoot && focused != DATA.Rooty.Root) {
       	Atom delete_atom = XInternAtom(DATA.Rooty.Display, "WM_DELETE_WINDOW", False);
		Atom protocols_atom = XInternAtom(DATA.Rooty.Display, "WM_PROTOCOLS", False);
        Atom* protocols = NULL;
        int n = 0;
        int supports_delete = 0;

        if(XGetWMProtocols(DATA.Rooty.Display, focused, &protocols, &n)) {
            while(n--) {
                if(protocols[n] == delete_atom) {
                    supports_delete = 1;
                    break;
                }
            }
            if(protocols) { XFree(protocols); }
        }

        if(supports_delete) {
            XEvent ev;
            ev.type = ClientMessage;
            ev.xclient.window = focused;
            ev.xclient.message_type = protocols_atom;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = delete_atom;
            ev.xclient.data.l[1] = CurrentTime;
            XSendEvent(DATA.Rooty.Display, focused, False, NoEventMask, &ev);
        } else {
			TraceLog("Couldn't close window with id: %lu", focused);
        }
    }
}

void KillFocused(void) {
	Window focused;
    int revert_to;
    XGetInputFocus(DATA.Rooty.Display, &focused, &revert_to);

    if(focused != None && focused != PointerRoot && focused != DATA.Rooty.Root) {
        XKillClient(DATA.Rooty.Display, focused);
    }
}

pid_t SpawnArrFree(char** command) {
    pid_t pid = fork();

    if(pid < 0) {
		TraceLog("fork() failed");
        return -1;
    }

    if(pid == 0) {
		setsid();
        execvp(command[0], command);
		TraceLog("execvp() failed");
		CleanUp();
        exit(EXIT_FAILURE);
    }

	return pid;
}

pid_t SpawnFree(int argvCount, ...) {
    va_list va;
    va_start(va, argvCount);

    char* argv[argvCount + 1];

    int i = 0;

    char* command = va_arg(va, char*);
    argv[i++] = command;

    for(int j = 1; j < argvCount; j++) {
        argv[i++] = va_arg(va, char*);
    }

    argv[i] = NULL;

    pid_t pid = fork();

    if(pid < 0) {
		TraceLog("fork() failed");
        va_end(va);
        return -1;
    }

    if(pid == 0) {
		setsid();
        execvp(command, argv);
		TraceLog("execvp() failed");
		CleanUp();
        exit(EXIT_FAILURE);
    }

    va_end(va);

	return pid;
}

pid_t SpawnArr(char** command) {
    pid_t pid = fork();

    if(pid < 0) {
		TraceLog("fork() failed");
        return -1;
    }

    if(pid == 0) {
        execvp(command[0], command);
		TraceLog("execvp() failed");
		CleanUp();
        exit(EXIT_FAILURE);
    }

	return pid;
}

pid_t Spawn(int argvCount, ...) {
    va_list va;
    va_start(va, argvCount);

    char* argv[argvCount + 1];

    int i = 0;

    char* command = va_arg(va, char*);
    argv[i++] = command;

    for(int j = 1; j < argvCount; j++) {
        argv[i++] = va_arg(va, char*);
    }

    argv[i] = NULL;

    pid_t pid = fork();

    if(pid < 0) {
		TraceLog("fork() failed");
        va_end(va);
        return -1;
    }

    if(pid == 0) {
        execvp(command, argv);
		TraceLog("execvp() failed");
		CleanUp();
        exit(EXIT_FAILURE);
    }

    va_end(va);

	return pid;
}

// TODO: Make this temp mkdir_p be not temp
int mkdir_p(const char *path, mode_t mode) {
    char tmp[1024];
    char *p;

    snprintf(tmp, sizeof(tmp), "%s", path);

    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            if(mkdir(tmp, mode) != 0 && errno != EEXIST) {
                return -1;
			}
            *p = '/';
        }
    }

    if(mkdir(tmp, mode) != 0 && errno != EEXIST) { return -1; }

    return 0;
}

// NOTE: Wierd ahh pid getter from the internet
pid_t GetWindowPid(Window window) {
    Atom netWmPid = XInternAtom(DATA.Rooty.Display, "_NET_WM_PID", false);
    Atom cardinal = XInternAtom(DATA.Rooty.Display, "CARDINAL", false);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char *prop = NULL;
    pid_t pid = -1;

    if(XGetWindowProperty(DATA.Rooty.Display, window, netWmPid, 0, 1, false, cardinal, &actualType, &actualFormat, &nItems, &bytesAfter, &prop) == Success && prop) {
        pid = (pid_t)(*(unsigned long *)prop);
        XFree(prop);
    }

    return pid;
}
