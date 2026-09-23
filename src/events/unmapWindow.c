#include "./events.h"

#include "../types.h"


void UnmapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, unmap);
}
