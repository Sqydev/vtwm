#include "./events.h"

#include "../types.h"

#include <wlr/types/wlr_xdg_shell.h>

void CommitWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, commit);

	struct wlr_xdg_surface* base = window->toplevel->base;

	if(base->initial_commit) {
		wlr_xdg_surface_schedule_configure(base);
	}
}