#ifndef VTWM_EVENTS_XDG_H
#define VTWM_EVENTS_XDG_H

#include "../../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct XdgEvents {
	struct wl_listener new_toplevel;
	struct wl_listener new_popup;
} XdgEvents;

typedef struct VtwmView {
	int x;
	int y;

	struct wl_list link;
	struct wlr_xdg_toplevel* xdg_toplevel;
	struct wlr_scene_tree* scene_tree;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;

	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
} VtwmView;

void NewToplevel(struct wl_listener* listener, void* data);

void NewPopup(struct wl_listener* listener, void* data);

void MapToplevel(struct wl_listener* listener, void* data);
void UnmapToplevel(struct wl_listener* listener, void* data);

void CommitToplevel(struct wl_listener* listener, void* data);

void DestroyToplevel(struct wl_listener* listener, void* data);

void RequestToplevelMaximize(struct wl_listener* listener, void* data);
void RequestToplevelFullscreen(struct wl_listener* listener, void* data);

#endif
