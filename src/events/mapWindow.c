#include "./events.h"

#include "../coredata.h"
#include "../types.h"

void MapWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, map);

    struct wlr_keyboard* keyboard = DATA.compositor.keyboard;

    wlr_seat_keyboard_notify_enter(DATA.compositor.seat, window->toplevel->base->surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    wlr_xdg_toplevel_set_activated(window->toplevel, true);
}
