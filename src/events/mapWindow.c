#include "./events.h"
#include "../types.h"

#include <stdio.h>

void MapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, map);

	printf("Window mapped: %s\n", window->toplevel->title ? window->toplevel->title : "(no title)");
}
