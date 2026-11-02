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

#include "./sections.h"

#include "../../coredata.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void ClearKeybinds(void) {
	for(size_t i = 0; i < CONFIG.keybindCount; i++) {
		free(CONFIG.keybinds[i].binding);
		free(CONFIG.keybinds[i].key);
		free(CONFIG.keybinds[i].action);
	}

	free(CONFIG.keybinds);
	CONFIG.keybinds = NULL;
	CONFIG.keybindCount = 0;
}

static void KeybindAdd(ConfigKeybind keybind) {
	ConfigKeybind* keybinds = realloc(
		CONFIG.keybinds,
		(CONFIG.keybindCount + 1) * sizeof(ConfigKeybind)
	);

	if(!keybinds) {
		free(keybind.binding);
		free(keybind.key);
		free(keybind.action);
		Panic(EXIT_FAILURE, "Failed to allocate keybind\n");
	}

	CONFIG.keybinds = keybinds;
	CONFIG.keybinds[CONFIG.keybindCount] = keybind;
	CONFIG.keybindCount++;
}

static void ParseKeybindBinding(char* binding, ConfigKeybind* keybind, size_t entryLine) {
	const char* cursor = binding;

	for(;;) {
		const char* separator = strstr(cursor, "&&");
		const char* end = separator ? separator : cursor + strlen(cursor);

		while(cursor < end && isspace((unsigned char)*cursor)) {
			cursor++;
		}

		while(end > cursor && isspace((unsigned char)end[-1])) {
			end--;
		}

		if(end > cursor) {
			char* part = strndup(cursor, (size_t)(end - cursor));
			if(!part) {
				Panic(EXIT_FAILURE, "Failed to allocate keybind part\n");
			}

			uint32_t modifier = ParseModifierName(part);

			if(modifier) {
				keybind->modifiers |= modifier;
				free(part);
			}
			else if(!keybind->key) {
				keybind->key = part;
			}
			else {
				ConfigWarning(entryLine, "Multiple key expressions in keybind, ignoring extra");
				free(part);
			}
		}

		if(!separator) { break; }

		cursor = separator + 2;
	}

	if(!keybind->key) {
		ConfigWarning(entryLine, "Missing key expression in keybind");
		keybind->key = DuplicateString("");
	}
}

static void ParseKeybindEntry(ConfigParser* parser) {
	Token first = parser->token;
	size_t entryLine = first.line;

	const char* bindingStart = first.data;
	const char* bindingEnd = first.data;

	bool foundArrow = false;

	while(parser->token.data) {
		if(TokenEquals(parser->token, "->")) {
			bindingEnd = parser->token.data;
			foundArrow = true;
			ParserAdvance(parser);
			break;
		}

		if(TokenEquals(parser->token, ";")) {
			ParserAdvance(parser);
			break;
		}

		if(TokenEquals(parser->token, "}")) { break; }

		bindingEnd = parser->token.data + parser->token.length;
		ParserAdvance(parser);
	}

	if(!foundArrow) {
		ConfigWarning(entryLine, "Expected '->' in keybind");
		return;
	}

	if(!parser->token.data ||
		TokenEquals(parser->token, ";") ||
		TokenEquals(parser->token, "}")) {
		ConfigWarning(entryLine, "Missing keybind action");
		ParserAccept(parser, ";");
		return;
	}

	const char* actionStart = parser->token.data;
	const char* actionEnd = actionStart;

	while(parser->token.data) {
		if(TokenEquals(parser->token, ";")) {
			actionEnd = parser->token.data;
			ParserAdvance(parser);
			break;
		}

		if(TokenEquals(parser->token, "}")) {
			actionEnd = parser->token.data;
			break;
		}

		actionEnd = parser->token.data + parser->token.length;
		ParserAdvance(parser);
	}

	if(actionEnd <= actionStart) {
		ConfigWarning(entryLine, "Missing keybind action");
		return;
	}

	ConfigKeybind keybind = {
		.binding = CopyTrimmed(bindingStart, bindingEnd),
		.layers = CONFIG_LAYER_ALL,
		.modifiers = 0,
		.key = NULL,
		.action = CopyTrimmed(actionStart, actionEnd)
	};

	ParseKeybindBinding(keybind.binding, &keybind, entryLine);

	KeybindAdd(keybind);
}

void ParseKeybindsBlock(ConfigParser* parser) {
	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		ParseKeybindEntry(parser);
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Keybinds");
}
