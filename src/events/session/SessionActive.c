#include "./session.h"

#include "../../coredata.h"

#include <wlr/backend/session.h>

void SessionActive(struct wl_listener* listener, void* data) {
	(void)listener;
	(void)data;

	COMP.session_active = COMP.session->active;
}
