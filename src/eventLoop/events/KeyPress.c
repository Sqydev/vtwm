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
#include "../../cleanup/cleanup.h"
#include "../../utils/utils.h"
#include "../../management/management.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>

void DoKeyPress(void) {
	for(size_t i = 0; i < DATA.Config.keybindsCount; i++) {
		if((DATA.events.xkey.state & DATA.Config.keybinds[i].mods) == DATA.Config.keybinds[i].mods && XLookupKeysym(&DATA.events.xkey, 0) == DATA.Config.keybinds[i].key) {
			if(DATA.Config.keybinds[i].actionsCount > 0 && DATA.Config.keybinds[i].actions && DATA.Config.keybinds[i].actions->argv) {
				for(size_t j = 0; j < DATA.Config.keybinds[i].actionsCount; j++) {
					if(strcmp(DATA.Config.keybinds[i].actions[j].argv[0], "exit") == 0) {
						CleanUp();
						exit(0);
					}
					else if(strcmp(DATA.Config.keybinds[i].actions[j].argv[0], "closefocused") == 0) {
						CloseFocused();
					}
					else if(strcmp(DATA.Config.keybinds[i].actions[j].argv[0], "killfocused") == 0) {
						KillFocused();
					}
					else if(strcmp(DATA.Config.keybinds[i].actions[j].argv[0], "workspace") == 0 && DATA.Config.keybinds[i].actions[j].argv[1]) {
						int workspace = atoi(DATA.Config.keybinds[i].actions[j].argv[1]);

						if(!DATA.Management.tilingOn) {
							SwitchToWorkspace(workspace);
						}
					}
					else {
						SpawnArrFree(DATA.Config.keybinds[i].actions[j].argv);
					}
				}
			}
		}
   	}
}
