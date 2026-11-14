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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

static bool KeybindMatchesKey(const char* binding, const char* keyName, int* rangeValue) {
	if(binding[0] != '(') {
		return strcasecmp(keyName, binding) == 0;
	}

	int first = 0;
	int last = 0;

	if(sscanf(binding, "(%d, ..., %d)", &first, &last) != 2 &&
		sscanf(binding, "(%d...%d)", &first, &last) != 2) {
		return false;
	}

	char* end = NULL;
	long value = strtol(keyName, &end, 10);

	if(end == keyName || *end != '\0') { return false; }
	if(value < first || value > last) { return false; }

	*rangeValue = (int)value;

	return true;
}

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

		int rangeValue = 0;

		if(!KeybindMatchesKey(keybind->key, keyName, &rangeValue)) {
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

		if(nameLength == 13 && strncasecmp(action, "gotoWorkspace", nameLength) == 0) {
			int workspace = 0;

			if(!ParseWorkspaceTarget(action, open, rangeValue, &workspace)) {
				wlr_log(WLR_ERROR, "Invalid gotoWorkspace action: %s\n", action);
				return true;
			}

			VtwmOutput* output = NULL;
			LayerName layer = LAYER_TILING;

			Window* window = seat && seat->focused_view ? seat->focused_view->window : NULL;

			if(window && window->workspace) {
				layer = window->layer;

				if(window->workspace->output) {
					output = window->workspace->output;
				}
				else {
					VtwmOutput* candidate = NULL;
					wl_list_for_each(candidate, &COMP.outputs, link) {
						if(candidate->workspace == window->workspace) {
							output = candidate;
							break;
						}
					}
				}
			}

			if(!output) {
				output = OutputGetActive();

				if(output && output->workspace) {
					layer = output->workspace->currentLayer;
				}
			}

			if(!output || !output->workspace || workspace < 0) {
				wlr_log(WLR_ERROR, "Cannot switch to workspace %d: no output available\n", workspace);
				return true;
			}

			size_t wdir = 0;

			if(!WorkspaceGetLocation(output->workspace, &wdir, NULL)) {
				wlr_log(WLR_ERROR, "Cannot switch to workspace %d: output workspace not found\n", workspace);
				return true;
			}

			SwitchToWorkspace(output, (size_t)workspace, wdir, layer);

			return true;
		}

		wlr_log(WLR_ERROR, "Unsupported keybind action: %s\n", action);
		return true;
	}

	return false;
}
