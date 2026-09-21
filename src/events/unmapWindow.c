#include "./events.h"

#include "../coredata.h"
#include "../types.h"

#include <stdio.h>

void UnmapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, unmap);

	if(DATA.TEMPFULLSCREENWINDOWTESTVAR == window) {
		DATA.TEMPFULLSCREENWINDOWTESTVAR = NULL;
	}
}
