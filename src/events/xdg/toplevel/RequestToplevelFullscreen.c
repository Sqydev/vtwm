#include "../xdg.h"

void RequestToplevelFullscreen(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmView* view = wl_container_of(listener, view, request_fullscreen);

	if(view->xdg_toplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
	}
}
