#include "./events.h"

#include "../coredata.h"
#include "../types.h"

void MapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, map);

	DATA.TEMPFULLSCREENWINDOWTESTVAR = window;

	wlr_xdg_toplevel_set_size(window->toplevel, DATA.compositor.output->output->width, DATA.compositor.output->output->height);
}
