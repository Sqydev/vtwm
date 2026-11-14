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

#include "../input.h"

#include "../../../coredata.h"
#include "../../../utils/utils.h"
#include "../../xdg/xdg.h"
#include "../../../wm/workspace/workspace.h"
#include "../../output/output.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

bool KeybindHandle(VtwmSeat* seat, xkb_keysym_t sym, uint32_t modifiers) {
	ConfigModifiers configModifiers = {0};

	if(modifiers & WLR_MODIFIER_SHIFT) {
		configModifiers.enabled[CONFIG_MODIFIER_SHIFT] = true;
	}

	if(modifiers & WLR_MODIFIER_CTRL) {
		configModifiers.enabled[CONFIG_MODIFIER_CTRL] = true;
	}

	if(modifiers & WLR_MODIFIER_ALT) {
		configModifiers.enabled[CONFIG_MODIFIER_ALT] = true;
	}

	if(modifiers & WLR_MODIFIER_LOGO) {
		configModifiers.enabled[CONFIG_MODIFIER_SUPER] = true;
	}

	// SUPERSCARYHARDCODEDFORTESTINGFORNOWDONTFORGET: config keybinds, yk and... first output to focused one
	if(sym == XKB_KEY_1 || sym == XKB_KEY_2) {
		if(wl_list_empty(&COMP.outputs)) {
			return true;
		}

		VtwmOutput* output = wl_container_of(COMP.outputs.next, output, link);

		SwitchToWorkspace(output, sym == XKB_KEY_1 ? 1 : 2, 0, LAYER_TILING);
		return true;
	}

	for(size_t i = 0; i < CONFIG.keybindCount; i++) {
		ConfigKeybind* keybind = &CONFIG.keybinds[i];

		if(!ConfigModifiersEqual(&keybind->modifiers, &configModifiers)) {
			continue;
		}

		if(!keybind->key || keybind->key[0] == '\0') {
			continue;
		}

		char keyName[64];
		if(xkb_keysym_get_name(sym, keyName, sizeof(keyName)) <= 0) {
			continue;
		}

		if(strcasecmp(keyName, keybind->key) != 0) {
			continue;
		}

		const char* action = keybind->action;
		if(!action || action[0] == '\0') {
			return true;
		}

		const char* open = strchr(action, '(');
		size_t nameLength = open ? (size_t)(open - action) : strlen(action);

		while(nameLength > 0 && isspace((unsigned char)action[nameLength - 1])) {
			nameLength--;
		}

		if(nameLength == 4 && strncasecmp(action, "quit", nameLength) == 0) {
			Close(0);
			return true;
		}

		if(nameLength == 12 && strncasecmp(action, "closeFocused", nameLength) == 0) {
			if(seat && seat->focused_view && seat->focused_view->xdg_toplevel) {
				wlr_xdg_toplevel_send_close(seat->focused_view->xdg_toplevel);
			}

			return true;
		}

		if(nameLength == 5 && strncasecmp(action, "spawn", nameLength) == 0) {
			if(!open) {
				wlr_log(WLR_ERROR, "Keybind spawn action is missing arguments\n");
				return true;
			}

			const char* close = strrchr(action, ')');
			const char* argumentStart = open + 1;
			const char* argumentEnd = close && close > argumentStart ? close : action + strlen(action);

			while(argumentStart < argumentEnd && isspace((unsigned char)argumentStart[0])) {
				argumentStart++;
			}

			while(argumentEnd > argumentStart && isspace((unsigned char)argumentEnd[-1])) {
				argumentEnd--;
			}

			size_t argumentLength = (size_t)(argumentEnd - argumentStart);

			if(argumentLength == 0) {
				wlr_log(WLR_ERROR, "Keybind spawn action has no command\n");
				return true;
			}

			if(argumentLength == sizeof("Terminal.command") - 1 &&
				strncasecmp(argumentStart, "Terminal.command", argumentLength) == 0) {
				Spawn(CONFIG.terminalCommandSliced);
			}
			else {
				SpawnCommandRange(argumentStart, argumentLength);
			}

			return true;
		}

		wlr_log(WLR_ERROR, "Unsupported keybind action: %s\n", action);
		return true;
	}

	return false;
}
