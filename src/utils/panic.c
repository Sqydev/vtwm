#include "./utils.h"

#include "../compositor/compositor.h"
#include "../coredata.h"
#include "../defines.h"

#include "../linclude/stb_image.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void Panic(int exitCode, const char* msg, ...) {
	va_list va;
	va_start(va, msg);

	if(msg) {
		vfprintf(stderr, msg, va);
		putc('\n', stderr);
	}

	va_end(va);

	FreeCompositor(&DATA.compositor);

	int width;
	int height;
	int channels;

	char* path = GetRandomFileFromDir(JoinStrings(VTWM_ASSETS_DIR, "panic/"));
	if(!path) {
		exit(exitCode);
	}

	unsigned char* img = stbi_load(path, &width, &height, &channels, 4);
	if(!img) {
		free(path);
		exit(exitCode);
	}

	free(path);

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
					printf("\033[0m");
					lastR = -1;
					lastG = -1;
					lastB = -1;
				}

				putchar(' ');
			}
			else {
				if(r != lastR || g != lastG || b != lastB) {
					printf("\033[48;2;%d;%d;%dm", r, g, b);

					lastR = r;
					lastG = g;
					lastB = b;
				}

				putchar(' ');
			}
		}

		if(lastR != -1) {
			printf("\033[0m");
			lastR = -1;
			lastG = -1;
			lastB = -1;
		}

		putchar('\n');
	}

	stbi_image_free(img);

	exit(exitCode);
}
