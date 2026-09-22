#include "../types.h"

#include <stdlib.h>

void DestroyWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, destroy);

	wl_list_remove(&window->destroy.link);
	wl_list_remove(&window->map.link);
	wl_list_remove(&window->unmap.link);
	wl_list_remove(&window->commit.link);

	free(window);
}
