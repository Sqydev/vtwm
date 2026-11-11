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
#include "../../utils/utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void ClearOutputRules(void) {
	ConfigClearOutputRules(&CONFIG.output.rules, &CONFIG.output.ruleCount);
}

static bool ParseResolutionToken(Token token, int* width, int* height) {
	const char* data = token.data;
	size_t length = token.length;

	size_t i = 0;

	for(int part = 0; part < 2; part++) {
		size_t start = i;

		while(i < length && isdigit((unsigned char)data[i])) {
			i++;
		}

		if(i == start) { return false; }

		size_t digits = i - start;

		if(digits > 9) { return false; }

		long number = 0;

		for(size_t j = start; j < i; j++) {
			number = number * 10 + (data[j] - '0');
		}

		if(part == 0) {
			*width = (int)number;
		}
		else {
			*height = (int)number;
		}

		if(part == 0) {
			if(i >= length || (data[i] != 'x' && data[i] != 'X')) { return false; }

			i++;
		}
	}

	return i == length;
}

static bool OutputRuleNameEquals(const ConfigOutputRule* rule, const char* name, bool forEach) {
	if(forEach) { return rule->name == NULL; }
	if(rule->name == NULL) { return false; }

	return strcmp(rule->name, name) == 0;
}

static void OutputRuleAdd(ConfigOutputRule rule, bool forEach, size_t entryLine) {
	for(size_t i = 0; i < CONFIG.output.ruleCount; i++) {
		if(OutputRuleNameEquals(&CONFIG.output.rules[i], rule.name, forEach)) {
			ConfigWarning(entryLine, "Duplicate Output rule, last one wins");
			break;
		}
	}

	ConfigOutputRule* rules = realloc(CONFIG.output.rules, (CONFIG.output.ruleCount + 1) * sizeof(ConfigOutputRule));

	if(!rules) {
		free(rule.name);
		Panic(EXIT_FAILURE, "Failed to allocate output rule\n");
	}

	CONFIG.output.rules = rules;
	CONFIG.output.rules[CONFIG.output.ruleCount] = rule;
	CONFIG.output.ruleCount++;
}

static void ParseOutputIntField(ConfigParser* parser, ConfigOutputValueType* type, int* value, const char* maxKeyword, const char* nextKeyword, size_t entryLine) {
	if(!parser->token.data || TokenEquals(parser->token, "}") || TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing value");
		ParserAccept(parser, ";");
		return;
	}

	Token token = parser->token;
	ParserAdvance(parser);

	if(maxKeyword && TokenEquals(token, maxKeyword)) {
		*type = CONFIG_OUTPUT_VALUE_MAX;
		*value = 0;
	}
	else if(nextKeyword && TokenEquals(token, nextKeyword)) {
		*type = CONFIG_OUTPUT_VALUE_NEXT;
		*value = 0;
	}
	else if(ParseIntegerToken(token, value)) {
		*type = CONFIG_OUTPUT_VALUE_NUMBER;
	}
	else {
		ConfigWarning(token.line, "Invalid value, ignoring");
	}

	ParserSkipSemicolon(parser, entryLine);
}

static void ParseOutputResField(ConfigParser* parser, ConfigOutputRule* rule, size_t entryLine) {
	if(!parser->token.data || TokenEquals(parser->token, "}") || TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing value");
		ParserAccept(parser, ";");
		return;
	}

	Token token = parser->token;
	ParserAdvance(parser);

	if(TokenEquals(token, "maxRes")) {
		rule->resType = CONFIG_OUTPUT_VALUE_MAX;
		rule->resWidth = 0;
		rule->resHeight = 0;
	}
	else if(ParseResolutionToken(token, &rule->resWidth, &rule->resHeight)) {
		rule->resType = CONFIG_OUTPUT_VALUE_NUMBER;
	}
	else {
		rule->resWidth = 0;
		rule->resHeight = 0;
		ConfigWarning(token.line, "Invalid value, ignoring");
	}

	ParserSkipSemicolon(parser, entryLine);
}

static void ParseOutputRuleBody(ConfigParser* parser, ConfigOutputRule* rule, bool forEach, size_t entryLine) {
	while(parser->token.data) {
		if(ParserAccept(parser, "}")) {
			OutputRuleAdd(*rule, forEach, entryLine);
			return;
		}

		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t fieldLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(key, "hz")) {
			if(ParserBeginValue(parser, fieldLine)) {
				ParseOutputIntField(parser, &rule->hzType, &rule->hz, "maxHz", NULL, fieldLine);
			}
		}
		else if(TokenEquals(key, "res")) {
			if(ParserBeginValue(parser, fieldLine)) {
				ParseOutputResField(parser, rule, fieldLine);
			}
		}
		else if(TokenEquals(key, "wdir")) {
			if(ParserBeginValue(parser, fieldLine)) {
				ParseOutputIntField(parser, &rule->wdirType, &rule->wdir, NULL, "wdir++", fieldLine);
			}
		}
		else if(TokenEquals(key, "workspace")) {
			if(ParserBeginValue(parser, fieldLine)) {
				ParseOutputIntField(parser, &rule->workspaceType, &rule->workspace, NULL, "workspace++", fieldLine);
			}
		}
		else {
			ConfigWarning(fieldLine, "Unknown Output rule field, ignoring");
			SkipUnknownEntry(parser, fieldLine);
		}
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Output rule");

	free(rule->name);
	rule->name = NULL;
}

static void ParseOutputRuleBlock(ConfigParser* parser, Token name, size_t entryLine) {
	ConfigOutputRule rule = {0};

	bool forEach = TokenEquals(name, "forEach");

	if(!forEach) {
		if(name.length == 0) {
			ConfigWarning(entryLine, "Missing output name, ignoring block");
			SkipBlockBody(parser, entryLine);
			return;
		}

		rule.name = TokenDuplicate(name);
	}

	ParseOutputRuleBody(parser, &rule, forEach, entryLine);
}

void ParseOutputBlock(ConfigParser* parser) {
	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t entryLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(parser->token, "{")) {
			ParserAdvance(parser);
			ParseOutputRuleBlock(parser, key, entryLine);
			ParserAccept(parser, ";");
			continue;
		}

		ConfigWarning(entryLine, "Unknown Output field, ignoring");
		SkipUnknownEntry(parser, entryLine);
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Output");
}
