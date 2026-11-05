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

#include "./categories.h"

#include "../values/values.h"

#include "../../coredata.h"

#include <stdlib.h>

void ClearStartUp(void) {
	ConfigClearStartUp(&CONFIG.startup);
}

static void StartUpAdd(char* command) {
	char** commands = realloc(
		CONFIG.startup.commands,
		(CONFIG.startup.commandCount + 1) * sizeof(char*)
	);

	if(!commands) {
		free(command);
		Panic(EXIT_FAILURE, "Failed to allocate startup command\n");
	}

	CONFIG.startup.commands = commands;
	CONFIG.startup.commands[CONFIG.startup.commandCount] = command;
	CONFIG.startup.commandCount++;
}

static void ParseStartUpEntry(ConfigParser* parser, Token name, size_t entryLine) {
	if(!TokenEquals(name, "spawn")) {
		ConfigWarning(entryLine, "Unknown StartUp entry, ignoring");
		SkipUnknownEntry(parser, entryLine);
		return;
	}

	if(!ParserExpect(parser, "(")) {
		SkipUnknownEntry(parser, entryLine);
		return;
	}

	if(!parser->token.data ||
		TokenEquals(parser->token, ")") ||
		TokenEquals(parser->token, "}") ||
		TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing spawn command");
		ParserAccept(parser, ")");
		ParserAccept(parser, ";");
		return;
	}

	char* command = TokenDuplicate(parser->token);
	ParserAdvance(parser);

	if(!ParserAccept(parser, ")")) {
		ConfigWarning(entryLine, "spawn expects exactly one argument");
		while(parser->token.data &&
			!TokenEquals(parser->token, ")") &&
			!TokenEquals(parser->token, ";") &&
			!TokenEquals(parser->token, "}")) {
			ParserAdvance(parser);
		}

		ParserAccept(parser, ")");
	}

	ParserSkipSemicolon(parser, entryLine);
	StartUpAdd(command);
}

void ParseStartUpBlock(ConfigParser* parser) {
	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		Token name = parser->token;
		size_t entryLine = name.line;
		ParserAdvance(parser);

		ParseStartUpEntry(parser, name, entryLine);
	}

	ConfigWarning(parser->line, "Unexpected end of file inside StartUp");
}
