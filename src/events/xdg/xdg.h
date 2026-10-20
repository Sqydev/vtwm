#ifndef VTWM_EVENTS_XDG_H
#define VTWM_EVENTS_XDG_H

#include <wayland-server-core.h>

typedef struct XdgEvents {
	struct wl_listener new_xdg_surface;
} XdgEvents;

void NewXdgSurface(struct wl_listener* listener, void* data);

#endif
