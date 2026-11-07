#ifndef VTWM_EVENTS_SESSION_INPUT_H
#define VTWM_EVENTS_SESSION_INPUT_H

#include <wayland-server-core.h>

typedef struct SessionEvents {
	struct wl_listener session_active_listener;
} SessionEvents;

void SessionActive(struct wl_listener* listener, void* data);

#endif
