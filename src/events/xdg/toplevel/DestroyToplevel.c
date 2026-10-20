#include "../xdg.h"

#include <stdlib.h>

void DestroyToplevel(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmView* view = wl_container_of(listener, view, destroy);

	wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->commit.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->request_maximize.link);
	wl_list_remove(&view->request_fullscreen.link);

	free(view);
}
