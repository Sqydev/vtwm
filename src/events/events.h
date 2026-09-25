#ifndef VTWM_EVENTS_H
#define VTWM_EVENTS_H

#include "../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct Events {
	struct wl_listener newOutput;
	struct wl_listener newToplevel;

	struct wl_listener newInput;
	struct wl_listener keyboardKey;
	struct wl_listener keyboardModifiers;
	struct wl_listener keyboardDestroy;
} Events;

void InitEvents(Compositor* compositor, Events* events);
void RemoveEvents(Events* events);

void NewOutput(struct wl_listener* listener, void* data);
void Frame(struct wl_listener* listener, void* data);

// When new window
void NewToplevel(struct wl_listener* listener, void* data);
void DestroyWindow(struct wl_listener* listener, void* data);

void MapWindow(struct wl_listener* listener, void* data);
void UnmapWindow(struct wl_listener* listener, void* data);
void CommitWindow(struct wl_listener* listener, void* data);

void NewInput(struct wl_listener* listener, void* data);
void KeyboardKey(struct wl_listener* listener, void* data);
void KeyboardModifiers(struct wl_listener* listener, void* data);
void KeyboardDestroy(struct wl_listener* listener, void* data);

#endif
