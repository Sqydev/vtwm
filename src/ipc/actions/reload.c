#include "./actions.h"

#include <wlr/util/log.h>

const char* ReloadConfigIpcAction(void) {
	wlr_log(WLR_DEBUG, "Got config reload command");

	if(!ReloadConfig()) {
		return "error";
	}

	UpdateCursorConfig();
	UpdateKeyboardConfig();

	return "ok";
}
