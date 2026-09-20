#include "../types.h"

#include <stdlib.h>

void DestroyWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, destroy);

	free(window);
}
