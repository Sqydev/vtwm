#include "../compositor/compositor.h"
#include "../coredata.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void Panic(const char* msg, int exitCode, ...) {
	va_list va;
	va_start(va, exitCode);

	if(msg) { vfprintf(stderr, msg, va); }

	va_end(va);

	FreeCompositor(&DATA.compositor);

	exit(exitCode);
}
