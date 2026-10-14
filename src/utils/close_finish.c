#include "./utils.h"

#include "../coredata.h"
#include "../compositor/compositor.h"

#include <stdlib.h>

static int closeExitCode = EXIT_SUCCESS;

void Close(int exitCode) {
	closeExitCode = exitCode;
	if(COMP.display) {
		wl_display_terminate(COMP.display);
	}
}


int Finish(void) {
	FreeCompositor();
	return closeExitCode;
}
