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

#include "./utils.h"

#include <ctype.h>

static bool IsAtLineStart(const char* data, size_t idx) {
	for(size_t i = idx; i > 0; i--) {
		unsigned char c = (unsigned char)data[i - 1];

		if(c == '\n') { return true; }
		if(!isspace(c)) { return false; }
	}

	return true;
}

static bool IsHashComment(const char* data, size_t size, size_t idx) {
	if(idx + 1 >= size) { return true; }

	unsigned char next = (unsigned char)data[idx + 1];

	if(isspace(next)) { return true; }

	return IsAtLineStart(data, idx);
}

static bool IsSinglePunctuation(char c) {
	switch(c) {
		case '{':
		case '}':
		case '(':
		case ')':
		case '[':
		case ']':
		case '=':
		case ';':
		case ',':
			return true;
		default: {
			return false;
		}
	}
}

static size_t MatchOperator(const char* data, size_t size, size_t idx) {
	if(idx + 2 < size && data[idx] == '.' && data[idx + 1] == '.' && data[idx + 2] == '.') {
		return 3;
	}

	if(idx + 1 < size) {
		if((data[idx] == '-' && data[idx + 1] == '>') || (data[idx] == '|' && data[idx + 1] == '|') || (data[idx] == '&' && data[idx + 1] == '&')) {
			return 2;
		}
	}

	return 0;
}

Token NextToken(const char* data, size_t size, size_t* idx, size_t* line) {
	Token token = {0};

	for(;;) {
		while(*idx < size && isspace((unsigned char)data[*idx])) {
			if(data[*idx] == '\n') {
				(*line)++;
			}

			(*idx)++;
		}

		if(*idx >= size) { return token; }

		if(data[*idx] == '/' && *idx + 1 < size && data[*idx + 1] == '/') {
			*idx += 2;

			while(*idx < size && data[*idx] != '\n') {
				(*idx)++;
			}

			continue;
		}

		if(data[*idx] == '#' && IsHashComment(data, size, *idx)) {
			while(*idx < size && data[*idx] != '\n') {
				(*idx)++;
			}

			continue;
		}

		break;
	}

	size_t start;
	size_t end;

	token.line = *line;

	if(data[*idx] == '"') {
		start = ++(*idx);

		while(*idx < size && data[*idx] != '"') {
			if(data[*idx] == '\n') {
				(*line)++;
			}

			(*idx)++;
		}

		end = *idx;

		if(*idx < size) { (*idx)++; }
	}
	else {
		size_t operatorLength = MatchOperator(data, size, *idx);

		if(operatorLength > 0) {
			start = *idx;
			*idx += operatorLength;
			end = *idx;
		}
		else if(IsSinglePunctuation(data[*idx])) {
			start = *idx;
			(*idx)++;
			end = *idx;
		}
		else {
			start = *idx;

			while(*idx < size && !isspace((unsigned char)data[*idx]) && !IsSinglePunctuation(data[*idx]) && MatchOperator(data, size, *idx) == 0) {
				(*idx)++;
			}

			end = *idx;
		}
	}

	token.data = data + start;
	token.length = end - start;

	return token;
}
