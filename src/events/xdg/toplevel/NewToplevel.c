#include "../xdg.h"

#include "../../../coredata.h"
#include "../../../utils/utils.h"

#include <stdlib.h>

#include <wlr/util/log.h>

void NewToplevel(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_xdg_toplevel* xdg_toplevel = data;

	VtwmView* view = IFNULLPANIC(calloc(1, sizeof(*view)), 1, "Failed to allocate view\n");

	view->xdg_toplevel = xdg_toplevel;
	view->scene_tree = IFNULLPANIC(wlr_scene_xdg_surface_create(&COMP.scene->tree, xdg_toplevel->base), 1, "Failed to create scene tree\n");

	view->scene_tree->node.data = view;
	xdg_toplevel->base->data = view->scene_tree;

	view->map.notify = MapToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &view->map);

	view->unmap.notify = UnmapToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &view->unmap);

	view->commit.notify = CommitToplevel;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &view->commit);

	view->destroy.notify = DestroyToplevel;
	wl_signal_add(&xdg_toplevel->events.destroy, &view->destroy);

	view->request_maximize.notify = RequestToplevelMaximize;
	wl_signal_add(&xdg_toplevel->events.request_maximize, &view->request_maximize);

	view->request_fullscreen.notify = RequestToplevelFullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &view->request_fullscreen);

	wlr_log(WLR_INFO, "New toplevel: %s (%s)", xdg_toplevel->title ? xdg_toplevel->title : "?", xdg_toplevel->app_id ? xdg_toplevel->app_id : "?");
}
