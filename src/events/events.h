#ifndef EVENTS_VTWM_H
#define EVENTS_VTWM_H

#include "../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct Events {
	struct wl_listener newOutput;
} Events;

void InitEvents(Compositor* compositor, Events* events);

void NewOutput(struct wl_listener* listener, void* data);
void Frame(struct wl_listener* listener, void* data);

#endif
