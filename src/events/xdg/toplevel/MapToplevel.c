#include "../xdg.h"

#include "../../../coredata.h"

void MapToplevel(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmView* view = wl_container_of(listener, view, map);

	wl_list_insert(&COMP.views, &view->link);
}
