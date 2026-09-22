#include "./events.h"

#include "../coredata.h"
#include "../types.h"

void MapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, map);

	DATA.TEMPFULLSCREENWINDOWTESTVAR = window;
}
