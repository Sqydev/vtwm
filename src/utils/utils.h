/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
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

#ifndef VTWM_UTILS_H
#define VTWM_UTILS_H

#include <stddef.h>
#include <stdbool.h>

void* IFNULLPANIC(void* value, int exitCode, const char* msg);
bool IFFALSEPANIC(bool value, int exitCode, const char* msg);
void IFNULLPANICALONE(void* value, int exitCode, const char* msg);
void IFFALSEPANICALONE(bool value, int exitCode, const char* msg);
const void* IFNULLPANICCONST(const void* value, int exitCode, const char* msg);
void IFNULLPANICALONECONST(const void* value, int exitCode, const char* msg);

void Close(int exitCode);
int Finish(void);

void FullWrite(int fd, const char* buffer, size_t size);

void InitPanic(void);
void Panic(int exitCode, const char* msg, ...)
	__attribute__((format(printf, 2, 3)));

char* GetRandomFileFromDir(const char* dirPath);

char* JoinStrings(const char* a, const char* b);

#endif
