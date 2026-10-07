#ifndef VTWM_EVENTS_H
#define VTWM_EVENTS_H

#include "../compositor/compositor.h"

#include <wayland-server-core.h>

typedef struct KeyboardHandler {
	struct wl_list link;
	struct wlr_keyboard* keyboard;
	struct wl_listener key;
	struct wl_listener modifiers;
	struct wl_listener destroy;
} KeyboardHandler;

typedef struct Events {
	struct wl_listener newOutput;
	struct wl_listener newToplevel;

	struct wl_listener newInput;
	struct wl_list keyboardHandlers;

	struct wl_listener pointerMotion;
	struct wl_listener pointerMotionAbsolute;
	struct wl_listener pointerButton;
	struct wl_listener pointerAxis;
	struct wl_listener pointerFrame;

	struct wl_listener requestSetCursor;
	struct wl_listener pointerFocusChange;
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

void PointerMotion(struct wl_listener* listener, void* data);
void PointerMotionAbsolute(struct wl_listener* listener, void* data);
void PointerButton(struct wl_listener* listener, void* data);
void PointerAxis(struct wl_listener* listener, void* data);
void PointerFrame(struct wl_listener* listener, void* data);
void RequestSetCursor(struct wl_listener* listener, void* data);
void PointerFocusChange(struct wl_listener* listener, void* data);

void ProcessCursorMotion(uint32_t time);

#endif
