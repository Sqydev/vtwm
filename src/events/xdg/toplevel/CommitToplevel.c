#include "../xdg.h"

void CommitToplevel(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmView* view = wl_container_of(listener, view, commit);

	if(view->xdg_toplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(view->xdg_toplevel, 0, 0);
	}
}
