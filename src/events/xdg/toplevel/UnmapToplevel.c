#include "../xdg.h"

void UnmapToplevel(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmView* view = wl_container_of(listener, view, unmap);

	wl_list_remove(&view->link);
}
