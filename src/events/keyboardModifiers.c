#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_keyboard.h>

void KeyboardModifiers(struct wl_listener* listener, void* data) {
    (void)listener;
	(void)data;

    struct wlr_keyboard* keyboard = DATA.compositor.keyboard;
    wlr_seat_keyboard_notify_modifiers(DATA.compositor.seat, &keyboard->modifiers);
}
