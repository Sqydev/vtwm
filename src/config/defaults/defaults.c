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

#include "./defaults.h"

#include "../../coredata.h"
#include "../../utils/utils.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_TERMINAL_COMMAND "alacritty"
#define DEFAULT_CURSOR_ICON "default"
#define DEFAULT_INPUT_LAYOUT "en"
#define DEFAULT_INPUT_REPEAT_RATE 25
#define DEFAULT_INPUT_REPEAT_DELAY 600
#define DEFAULT_BORDER_FOCUSED_COLOR 0xFFFFFFu
#define DEFAULT_BORDER_UNFOCUSED_COLOR 0x000000u
#define DEFAULT_BORDER_RADIUS 4

void ClearConfigValues(void) {
	free(CONFIG.terminalCommand);
	CONFIG.terminalCommand = NULL;

	FreeStringArray(CONFIG.terminalCommandSliced);
	CONFIG.terminalCommandSliced = NULL;

	memset(CONFIG.looks, 0, sizeof(CONFIG.looks));

	free(CONFIG.cursor.icon);
	CONFIG.cursor.icon = NULL;
	CONFIG.cursor.hardware = false;

	free(CONFIG.input.layout);
	CONFIG.input.layout = NULL;
	CONFIG.input.repeatRate = 0;
	CONFIG.input.repeatDelay = 0;

	for(size_t i = 0; i < CONFIG.startup.commandCount; i++) {
		free(CONFIG.startup.commands[i]);
	}

	free(CONFIG.startup.commands);
	CONFIG.startup.commands = NULL;
	CONFIG.startup.commandCount = 0;

	for(size_t i = 0; i < CONFIG.keybindCount; i++) {
		free(CONFIG.keybinds[i].binding);
		free(CONFIG.keybinds[i].key);
		free(CONFIG.keybinds[i].action);
	}

	free(CONFIG.keybinds);
	CONFIG.keybinds = NULL;
	CONFIG.keybindCount = 0;
}

void SetConfigDefaults(void) {
	ClearConfigValues();

	CONFIG.terminalCommand = IFNULLPANIC(
		strdup(DEFAULT_TERMINAL_COMMAND),
		EXIT_FAILURE,
		"Failed to allocate default config string\n"
	);

	CONFIG.cursor.icon = IFNULLPANIC(
		strdup(DEFAULT_CURSOR_ICON),
		EXIT_FAILURE,
		"Failed to allocate default config string\n"
	);
	CONFIG.cursor.hardware = false;

	CONFIG.input.layout = IFNULLPANIC(
		strdup(DEFAULT_INPUT_LAYOUT),
		EXIT_FAILURE,
		"Failed to allocate default config string\n"
	);
	CONFIG.input.repeatRate = DEFAULT_INPUT_REPEAT_RATE;
	CONFIG.input.repeatDelay = DEFAULT_INPUT_REPEAT_DELAY;

	ConfigLooks looks = {
		.defined = true,
		.margin = {
			.top = 0,
			.bottom = 0,
			.left = 0,
			.right = 0
		},
		.border = {
			.focusedColor = DEFAULT_BORDER_FOCUSED_COLOR,
			.unFocusedColor = DEFAULT_BORDER_UNFOCUSED_COLOR,
			.radius = DEFAULT_BORDER_RADIUS
		}
	};

	CONFIG.looks[CONFIG_LAYER_NORMAL] = looks;
	CONFIG.looks[CONFIG_LAYER_SUPER] = looks;
}
