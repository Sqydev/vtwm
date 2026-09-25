#include "../coredata.h"
#include "../compositor/compositor.h"

#include <stdlib.h>

static int closeExitCode = EXIT_SUCCESS;

void Close(int exitCode) {
    closeExitCode = exitCode;
    if(DATA.compositor.display) {
        wl_display_terminate(DATA.compositor.display);
    }
}


int Finish(void) {
    FreeCompositor(&DATA.compositor);
    return closeExitCode;
}
