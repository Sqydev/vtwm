#include "./close.h"

#include "../../coredata.h"

void Close(int exitCode) {
	DATA.exitCode = exitCode;
	if(COMP.display) {
		wl_display_terminate(COMP.display);
	}
}
