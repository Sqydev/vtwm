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

#ifndef VTWM_CONFIG_PARSER_H
#define VTWM_CONFIG_PARSER_H

#include "../config.h"

#include "../../utils/utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ConfigParser {
	const char* data;
	size_t size;
	size_t idx;
	size_t line;
	Token token;
} ConfigParser;

void ConfigParserInit(ConfigParser* parser, const char* data, size_t size);
void ParserAdvance(ConfigParser* parser);
bool ParserAccept(ConfigParser* parser, const char* text);
bool ParserExpect(ConfigParser* parser, const char* text);
void ParserSkipSemicolon(ConfigParser* parser, size_t entryLine);

void ConfigWarning(size_t line, const char* message);

void SkipBlockBody(ConfigParser* parser, size_t entryLine);
void SkipBlock(ConfigParser* parser, size_t entryLine);
void SkipUnknownEntry(ConfigParser* parser, size_t entryLine);

bool ParserBeginValue(ConfigParser* parser, size_t entryLine);

void ParseStringValue(ConfigParser* parser, char** destination, size_t entryLine);
void ParseIntegerValue(ConfigParser* parser, int* destination, size_t entryLine);
void ParseBooleanValue(ConfigParser* parser, bool* destination, size_t entryLine);
void ParseColorValue(ConfigParser* parser, uint32_t* destination, size_t entryLine);

bool ParseLayerName(Token token, ConfigLayers* layers);
ConfigLayers ParseLayerScope(ConfigParser* parser);
bool ParseModifierName(const char* name, ConfigModifier* modifier);

#endif
