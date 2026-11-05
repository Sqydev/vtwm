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

#include "./parse.h"

#include <stddef.h>
#include <stdint.h>

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
