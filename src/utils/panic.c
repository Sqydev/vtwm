#include "./utils.h"

#include "../compositor/compositor.h"
#include "../coredata.h"
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
		int n = write(STDERR_FILENO, panicScreen, panicScreenSize);
		(void)n;
	}

	raise(sig);
}

void InitPanic(void) {
	InitPanicScreen();

	struct sigaction sa = {0};

	sa.sa_handler = PanicSignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NODEFER;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGBUS,  &sa, NULL);
	sigaction(SIGILL,  &sa, NULL);
	sigaction(SIGFPE,  &sa, NULL);
}

void Panic(int exitCode, const char* msg, ...) {
	va_list va;
	va_start(va, msg);

	if(msg) {
		vfprintf(stderr, msg, va);
		putc('\n', stderr);
	}

	va_end(va);

	if(!panicFromSig) {
		FreeCompositor(&DATA.compositor);
	}

	if(panicScreen && panicScreenSize) {
		fwrite(panicScreen, 1, panicScreenSize, stderr);
		fflush(stderr);
	}

	if(!panicFromSig) {
		exit(exitCode);
	}
}
