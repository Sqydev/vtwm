#include "./events.h"
#include "../types.h"

#include <stdio.h>

void UnmapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, unmap);

	printf("Window unmapped: %s\n", window->toplevel->title ? window->toplevel->title : "(no title)");
}
