#ifndef VTWM_EVENTS_XDG_H
#define VTWM_EVENTS_XDG_H

#include "../../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct XdgEvents {
	struct wl_listener new_xdg_surface;
} XdgEvents;

typedef struct VtwmView {
	Compositor* server;

	int x;
	int y;

	struct wl_list link;
	struct wlr_xdg_toplevel* xdg_toplevel;
	struct wlr_scene_node* scene_node;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;

	struct wl_listener request_move;
	struct wl_listener request_resize;
} VtwmView;

void NewXdgSurface(struct wl_listener* listener, void* data);

#endif
