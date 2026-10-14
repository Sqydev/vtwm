#include <stdlib.h>

static int closeExitCode = EXIT_SUCCESS;

void Close(int exitCode) {
	(void)exitCode;
	return;
}


int Finish(void) {
	return closeExitCode;
}
