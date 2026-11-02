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

#include "./parser.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void ConfigParserInit(ConfigParser* parser, const char* data, size_t size) {
	parser->data = data;
	parser->size = size;
	parser->idx = 0;
	parser->line = 1;
	parser->token = (Token){0};

	ParserAdvance(parser);
}

void ParserAdvance(ConfigParser* parser) {
	parser->token = NextToken(parser->data, parser->size, &parser->idx, &parser->line);
}

bool ParserAccept(ConfigParser* parser, const char* text) {
	if(TokenEquals(parser->token, text)) {
		ParserAdvance(parser);
		return true;
	}

	return false;
}

bool ParserExpect(ConfigParser* parser, const char* text) {
	if(ParserAccept(parser, text)) { return true; }

	char message[128];
	snprintf(message, sizeof(message), "Expected '%s'", text);

	ConfigWarning(parser->token.data ? parser->token.line : parser->line, message);

	return false;
}

void ParserSkipSemicolon(ConfigParser* parser, size_t entryLine) {
	if(!ParserAccept(parser, ";")) {
		ConfigWarning(entryLine, "Expected ';' after value");
	}
}

void ConfigWarning(size_t line, const char* message) {
	fprintf(stderr, "Config warning at line %zu: %s\n", line, message);
}

char* DuplicateString(const char* value) {
	char* copy = strdup(value);
	if(!copy) {
		Panic(EXIT_FAILURE, "Failed to allocate config string\n");
	}

	return copy;
}

char* TokenDuplicate(Token token) {
	char* copy = strndup(token.data, token.length);
	if(!copy) {
		Panic(EXIT_FAILURE, "Failed to allocate config token\n");
	}

	return copy;
}

char* CopyTrimmed(const char* start, const char* end) {
	while(start < end && isspace((unsigned char)*start)) {
		start++;
	}

	while(end > start && isspace((unsigned char)end[-1])) {
		end--;
	}

	size_t length = (size_t)(end - start);

	char* copy = malloc(length + 1);
	if(!copy) {
		Panic(EXIT_FAILURE, "Failed to allocate config string\n");
	}

	memcpy(copy, start, length);
	copy[length] = '\0';

	return copy;
}

void SkipBlockBody(ConfigParser* parser, size_t entryLine) {
	int depth = 1;

	while(parser->token.data) {
		if(TokenEquals(parser->token, "{")) {
			depth++;
			ParserAdvance(parser);
			continue;
		}

		if(TokenEquals(parser->token, "}")) {
			depth--;
			ParserAdvance(parser);

			if(depth == 0) { return; }
			continue;
		}

		ParserAdvance(parser);
	}

	ConfigWarning(entryLine, "Unexpected end of file inside block");
}

void SkipBlock(ConfigParser* parser, size_t entryLine) {
	if(!ParserExpect(parser, "{")) { return; }

	SkipBlockBody(parser, entryLine);
}

void SkipUnknownEntry(ConfigParser* parser, size_t entryLine) {
	if(ParserAccept(parser, "=")) {
		if(parser->token.data &&
			!TokenEquals(parser->token, "}") &&
			!TokenEquals(parser->token, ";")) {
			ParserAdvance(parser);
		}

		ParserAccept(parser, ";");
		return;
	}

	if(TokenEquals(parser->token, "{")) {
		SkipBlock(parser, entryLine);
		ParserAccept(parser, ";");
		return;
	}

	if(TokenEquals(parser->token, "->")) {
		while(parser->token.data &&
			!TokenEquals(parser->token, "{") &&
			!TokenEquals(parser->token, ";")) {
			ParserAdvance(parser);
		}

		if(TokenEquals(parser->token, "{")) {
			SkipBlock(parser, entryLine);
		}

		ParserAccept(parser, ";");
		return;
	}

	while(parser->token.data &&
		!TokenEquals(parser->token, ";") &&
		!TokenEquals(parser->token, "}")) {
		ParserAdvance(parser);
	}

	ParserAccept(parser, ";");
}

bool ParserBeginValue(ConfigParser* parser, size_t entryLine) {
	if(ParserAccept(parser, "=")) { return true; }

	ConfigWarning(entryLine, "Expected '=' after field name");
	SkipUnknownEntry(parser, entryLine);

	return false;
}

bool ParseIntegerToken(Token token, int* destination) {
	char* string = TokenDuplicate(token);

	char* end = NULL;

	errno = 0;
	long number = strtol(string, &end, 10);

	bool valid = (errno == 0 && end != string && *end == '\0' &&
		number >= INT_MIN && number <= INT_MAX);

	free(string);

	if(!valid) { return false; }

	*destination = (int)number;

	return true;
}

static int HexValue(char c) {
	if(c >= '0' && c <= '9') { return c - '0'; }
	if(c >= 'a' && c <= 'f') { return c - 'a' + 10; }
	if(c >= 'A' && c <= 'F') { return c - 'A' + 10; }

	return -1;
}

bool ParseColorToken(Token token, uint32_t* destination) {
	if(!token.data || token.length < 2 || token.data[0] != '#') {
		return false;
	}

	size_t digits = token.length - 1;
	uint32_t value = 0;

	if(digits == 3) {
		for(size_t i = 0; i < 3; i++) {
			int digit = HexValue(token.data[i + 1]);
			if(digit < 0) { return false; }

			value = (value << 4) | (uint32_t)digit;
		}

		uint32_t red = (value >> 8) & 0xF;
		uint32_t green = (value >> 4) & 0xF;
		uint32_t blue = value & 0xF;

		value = ((red * 0x11u) << 16) | ((green * 0x11u) << 8) | (blue * 0x11u);
	}
	else if(digits == 6) {
		for(size_t i = 0; i < 6; i++) {
			int digit = HexValue(token.data[i + 1]);
			if(digit < 0) { return false; }

			value = (value << 4) | (uint32_t)digit;
		}
	}
	else {
		return false;
	}

	*destination = value;

	return true;
}

void ParseStringValue(ConfigParser* parser, char** destination, size_t entryLine) {
	if(!parser->token.data ||
		TokenEquals(parser->token, "}") ||
		TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing string value");
		ParserAccept(parser, ";");
		return;
	}

	if(parser->token.length == 0) {
		ConfigWarning(entryLine, "Empty string value, keeping previous value");
		ParserAdvance(parser);
		ParserSkipSemicolon(parser, entryLine);
		return;
	}

	char* value = TokenDuplicate(parser->token);
	ParserAdvance(parser);

	free(*destination);
	*destination = value;

	ParserSkipSemicolon(parser, entryLine);
}

void ParseIntegerValue(ConfigParser* parser, int* destination, size_t entryLine) {
	if(!parser->token.data ||
		TokenEquals(parser->token, "}") ||
		TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing integer value");
		ParserAccept(parser, ";");
		return;
	}

	Token value = parser->token;
	ParserAdvance(parser);

	if(!ParseIntegerToken(value, destination)) {
		ConfigWarning(value.line, "Invalid integer value");
	}

	ParserSkipSemicolon(parser, entryLine);
}

void ParseBooleanValue(ConfigParser* parser, bool* destination, size_t entryLine) {
	if(!parser->token.data ||
		TokenEquals(parser->token, "}") ||
		TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing boolean value");
		ParserAccept(parser, ";");
		return;
	}

	Token value = parser->token;
	ParserAdvance(parser);

	if(TokenEquals(value, "true")) {
		*destination = true;
	}
	else if(TokenEquals(value, "false")) {
		*destination = false;
	}
	else {
		ConfigWarning(value.line, "Invalid boolean value");
	}

	ParserSkipSemicolon(parser, entryLine);
}

void ParseColorValue(ConfigParser* parser, uint32_t* destination, size_t entryLine) {
	if(!parser->token.data ||
		TokenEquals(parser->token, "}") ||
		TokenEquals(parser->token, ";")) {
		ConfigWarning(entryLine, "Missing color value");
		ParserAccept(parser, ";");
		return;
	}

	Token value = parser->token;
	ParserAdvance(parser);

	if(!ParseColorToken(value, destination)) {
		ConfigWarning(value.line, "Invalid color value");
	}

	ParserSkipSemicolon(parser, entryLine);
}

bool TokenEqualsIgnoreCase(Token token, const char* text) {
	size_t length = strlen(text);

	return token.data && token.length == length &&
		strncasecmp(token.data, text, length) == 0;
}

bool ParseLayerName(Token token, uint32_t* layers) {
	if(TokenEqualsIgnoreCase(token, "TTY")) {
		*layers |= 1u << CONFIG_LAYER_TTY;
		return true;
	}

	if(TokenEqualsIgnoreCase(token, "NORMAL")) {
		*layers |= 1u << CONFIG_LAYER_NORMAL;
		return true;
	}

	if(TokenEqualsIgnoreCase(token, "SUPER")) {
		*layers |= 1u << CONFIG_LAYER_SUPER;
		return true;
	}

	return false;
}

uint32_t ParseLayerScope(ConfigParser* parser) {
	uint32_t layers = 0;

	if(!ParserExpect(parser, "(")) {
		return CONFIG_LAYER_ALL;
	}

	while(parser->token.data) {
		if(ParserAccept(parser, ")")) { break; }
		if(ParserAccept(parser, "||")) { continue; }

		if(!ParseLayerName(parser->token, &layers)) {
			ConfigWarning(parser->token.line, "Unknown layer name");
		}

		ParserAdvance(parser);
	}

	return layers ? layers : CONFIG_LAYER_ALL;
}

uint32_t ParseModifierName(const char* name) {
	if(strcasecmp(name, "SUPER") == 0 ||
		strcasecmp(name, "WIN") == 0 ||
		strcasecmp(name, "LOGO") == 0 ||
		strcasecmp(name, "MOD4") == 0) {
		return CONFIG_MODIFIER_SUPER;
	}

	if(strcasecmp(name, "CTRL") == 0 ||
		strcasecmp(name, "CONTROL") == 0) {
		return CONFIG_MODIFIER_CTRL;
	}

	if(strcasecmp(name, "ALT") == 0 || strcasecmp(name, "MOD1") == 0) {
		return CONFIG_MODIFIER_ALT;
	}

	if(strcasecmp(name, "SHIFT") == 0) {
		return CONFIG_MODIFIER_SHIFT;
	}

	return 0;
}
