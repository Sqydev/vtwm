#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_keyboard.h>

void KeyboardModifiers(struct wl_listener* listener, void* data) {
	(void)data;
	KeyboardHandler* handler = wl_container_of(listener, handler, modifiers);
	wlr_seat_keyboard_notify_modifiers(DATA.compositor.seat, &handler->keyboard->modifiers);
}
