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

#include "./utils.h"

#include "../defines.h"

#include "../linclude/stb_image.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>

// I know, I know. It's complicated for just a simple Panic that's hooked to sigs. But it just renders the output before panic because it can stop and do DEADLOCK and I won't explain why and what it is because this commnet is arleady big enough

static volatile sig_atomic_t panicFromSig = 0;

static char* panicScreen = NULL;
static size_t panicScreenSize = 0;

void InitPanicScreen(void) {
	int width;
	int height;
	int channels;

	char* dir = JoinStrings(VTWM_ASSETS_DIR, "panic/");
	if(!dir) {
		return;
	}

	char* path = GetRandomFileFromDir(dir);
	free(dir);

	if(!path) {
		return;
	}

	unsigned char* img = stbi_load(path, &width, &height, &channels, 4);
	free(path);

	if(!img) {
		return;
	}

	size_t rows = (height + 1) / 2;
	size_t pixels = rows * (size_t)width;

	size_t capacity = pixels * 23 + rows + 1;

	char* buffer = malloc(capacity);
	if(!buffer) {
		stbi_image_free(img);
		return;
	}

	size_t pos = 0;

	int lastR = -1;
	int lastG = -1;
	int lastB = -1;

	for(int y = 0; y < height; y += 2) {
		for(int x = 0; x < width; x++) {
			int i = (y * width + x) * 4;

			unsigned char r = img[i];
			unsigned char g = img[i + 1];
			unsigned char b = img[i + 2];
			unsigned char a = img[i + 3];

			if(a == 0) {
				if(lastR != -1) {
					pos += snprintf(buffer + pos, capacity - pos, "\033[0m");

					lastR = -1;
					lastG = -1;
					lastB = -1;
				}

				buffer[pos++] = ' ';
			}
			else {
				if(r != lastR || g != lastG || b != lastB) {
					pos += snprintf(buffer + pos, capacity - pos, "\033[48;2;%u;%u;%um", r, g, b);

					lastR = r;
					lastG = g;
					lastB = b;
				}

				buffer[pos++] = ' ';
			}
		}

		if(lastR != -1) {
			pos += snprintf(buffer + pos, capacity - pos, "\033[0m");

			lastR = -1;
			lastG = -1;
			lastB = -1;
		}

		buffer[pos++] = '\n';
	}

	buffer[pos] = '\0';

	stbi_image_free(img);

	free(panicScreen);

	panicScreen = buffer;
	panicScreenSize = pos;
}

void PanicSignalHandler(int sig) {
	struct sigaction sa = {0};
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);

	sigaction(sig, &sa, NULL);

	panicFromSig = sig;

	if(panicScreen && panicScreenSize) {
		FullWrite(STDERR_FILENO, panicScreen, panicScreenSize);
	}

	raise(sig);
}

void InitPanic(void) {
	InitPanicScreen();

	struct sigaction sa = {0};

	sa.sa_handler = PanicSignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGBUS,  &sa, NULL);
	sigaction(SIGILL,  &sa, NULL);
	sigaction(SIGFPE,  &sa, NULL);
}

void vaPanic(int exitCode, const char* msg, va_list args) {
	fflush(stderr);

	if(msg) {
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wformat-nonliteral"
		vfprintf(stderr, msg, args);
		#pragma GCC diagnostic pop
		putc('\n', stderr);
	}

	if(panicScreen && panicScreenSize) {
		fwrite(panicScreen, 1, panicScreenSize, stderr);
		fflush(stderr);
	}

	if(!panicFromSig) {
		exit(exitCode);
	}
}

void Panic(int exitCode, const char* msg, ...) {
	va_list args;
	va_start(args, msg);

	vaPanic(exitCode, msg, args);

	va_end(args);
}
