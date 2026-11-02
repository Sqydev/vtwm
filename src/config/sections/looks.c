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

static void ParseMarginBlock(ConfigParser* parser, ConfigMargin* margin) {
	if(!ParserExpect(parser, "{")) { return; }

	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t entryLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(key, "top")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseIntegerValue(parser, &margin->top, entryLine);
			}
		}
		else if(TokenEquals(key, "bottom")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseIntegerValue(parser, &margin->bottom, entryLine);
			}
		}
		else if(TokenEquals(key, "left")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseIntegerValue(parser, &margin->left, entryLine);
			}
		}
		else if(TokenEquals(key, "right")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseIntegerValue(parser, &margin->right, entryLine);
			}
		}
		else {
			ConfigWarning(entryLine, "Unknown Margin field, ignoring");
			SkipUnknownEntry(parser, entryLine);
		}
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Margin");
}

static void ParseBorderBlock(ConfigParser* parser, ConfigBorder* border) {
	if(!ParserExpect(parser, "{")) { return; }

	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t entryLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(key, "focusedColor")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseColorValue(parser, &border->focusedColor, entryLine);
			}
		}
		else if(TokenEquals(key, "unFocusedColor")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseColorValue(parser, &border->unFocusedColor, entryLine);
			}
		}
		else if(TokenEquals(key, "radius")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseIntegerValue(parser, &border->radius, entryLine);
			}
		}
		else {
			ConfigWarning(entryLine, "Unknown Border field, ignoring");
			SkipUnknownEntry(parser, entryLine);
		}
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Border");
}

void ParseLooksBlock(ConfigParser* parser, uint32_t layers) {
	ConfigLooks looks = {
		.defined = true,
		.margin = {0},
		.border = {0}
	};

	for(size_t i = 0; i < CONFIG_LAYER_COUNT; i++) {
		if(layers & (1u << i)) {
			looks = CONFIG.looks[i];
			break;
		}
	}

	looks.defined = true;

	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { break; }
		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t entryLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(key, "Margin")) {
			ParseMarginBlock(parser, &looks.margin);
			ParserAccept(parser, ";");
		}
		else if(TokenEquals(key, "Border")) {
			ParseBorderBlock(parser, &looks.border);
			ParserAccept(parser, ";");
		}
		else {
			ConfigWarning(entryLine, "Unknown Looks field, ignoring");
			SkipUnknownEntry(parser, entryLine);
		}
	}

	if(!parser->token.data) {
		ConfigWarning(parser->line, "Unexpected end of file inside Looks");
	}

	for(size_t i = 0; i < CONFIG_LAYER_COUNT; i++) {
		if(layers & (1u << i)) {
			CONFIG.looks[i] = looks;
		}
	}
}
