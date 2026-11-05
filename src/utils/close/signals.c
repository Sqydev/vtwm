#include "./close.h"

#include "../../coredata.h"

#include <signal.h>
#include <wlr/util/log.h>

static int HandleExitSignal(int signalNumber, void* data) {
	(void)data;

	wlr_log(WLR_INFO, "Received signal %d, shutting down", signalNumber);
	Close(128 + signalNumber);

	return 0;
}

void InitCloseSignals(void) {
	if(!COMP.display) {
		return;
	}

	struct wl_event_loop* loop = wl_display_get_event_loop(COMP.display);
	if(!loop) {
		wlr_log(WLR_ERROR, "Failed to get event loop for signal handling");
		return;
	}

	const int signals[] = { SIGINT, SIGTERM, SIGQUIT };

	for(size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); i++) {
		DATA.exitSignalSources[i] = wl_event_loop_add_signal(loop, signals[i], HandleExitSignal, NULL);

		if(!DATA.exitSignalSources[i]) {
			wlr_log(WLR_ERROR, "Failed to add handler for signal %d", signals[i]);
		}
	}
}
