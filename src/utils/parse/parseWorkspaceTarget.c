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

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

bool ParseWorkspaceTarget(const char* action, const char* open, int rangeValue, int* workspace) {
	const char* close = strrchr(action, ')');

	if(!open || !close || close <= open) { return false; }

	const char* cursor = open + 1;

	while(cursor < close && isspace((unsigned char)*cursor)) {
		cursor++;
	}

	if(strncmp(cursor, "argv[1]", 7) == 0) {
		cursor += 7;

		while(cursor < close && isspace((unsigned char)*cursor)) {
			cursor++;
		}

		int offset = 0;

		if(cursor < close && (*cursor == '+' || *cursor == '-')) {
			char* end = NULL;
			long value = strtol(cursor + 1, &end, 10);

			if(end == cursor + 1) { return false; }

			offset = *cursor == '-' ? -(int)value : (int)value;
			cursor = end;
		}

		while(cursor < close && isspace((unsigned char)*cursor)) {
			cursor++;
		}

		if(cursor != close) { return false; }

		*workspace = rangeValue + offset;

		return *workspace >= 0;
	}

	char* end = NULL;
	long value = strtol(cursor, &end, 10);

	if(end == cursor) { return false; }

	while(end < close && isspace((unsigned char)*end)) {
		end++;
	}

	if(end != close) { return false; }
	if(value < 0 || value > INT_MAX) { return false; }

	*workspace = (int)value;

	return true;
}
