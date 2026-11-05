#include "./close.h"

#include "../../coredata.h"

int Finish(void) {
	FinishIPC();

	for(size_t i = 0; i < sizeof(DATA.exitSignalSources) / sizeof(DATA.exitSignalSources[0]); i++) {
		if(DATA.exitSignalSources[i]) {
			wl_event_source_remove(DATA.exitSignalSources[i]);
			DATA.exitSignalSources[i] = NULL;
		}
	}

	FreeCompositor();
	UnloadConfig();
	return DATA.exitCode;
}
