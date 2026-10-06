#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_pointer.h>

void PointerButton(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_pointer_button_event* event = data;

	wlr_seat_pointer_notify_button(DATA.compositor.seat, event->time_msec, event->button, event->state);
}
