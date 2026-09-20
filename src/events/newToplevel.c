#include "./events.h"

#include "../types.h"

#include <stdio.h>
#include <stdlib.h>

void NewToplevel(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_xdg_toplevel* toplevel = data;

	Window* window = calloc(1, sizeof(Window));
	if(!window) {
		fprintf(stderr, "Failed to allocate Window\n");
		return;
	}

	window->toplevel = toplevel;

	window->map.notify = MapWindow;
	wl_signal_add(&toplevel->base->surface->events.map, &window->map);

	window->unmap.notify = UnmapWindow;
	wl_signal_add(&toplevel->base->surface->events.unmap, &window->unmap);

	window->destroy.notify = DestroyWindow;
	wl_signal_add(&toplevel->events.destroy, &window->destroy);

	printf("New window: %s (%s)\n", toplevel->title ? toplevel->title : "(no title)", toplevel->app_id ? toplevel->app_id : "(no app_id)");
}
