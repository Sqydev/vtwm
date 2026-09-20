#ifndef VTWM_EVENTS_H
#define VTWM_EVENTS_H

#include "../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct Events {
	struct wl_listener newOutput;
	struct wl_listener newToplevel;
} Events;

void InitEvents(Compositor* compositor, Events* events);

void NewOutput(struct wl_listener* listener, void* data);
void Frame(struct wl_listener* listener, void* data);

// When new window
void NewToplevel(struct wl_listener* listener, void* data);
void DestroyWindow(struct wl_listener* listener, void* data);

void MapWindow(struct wl_listener* listener, void* data);
void UnmapWindow(struct wl_listener* listener, void* data);

#endif
