#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_seat.h>

void RequestSetCursor(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_seat_pointer_request_set_cursor_event* event = data;
	struct wlr_seat* seat = DATA.compositor.seat;

	if(seat->pointer_state.focused_client != event->seat_client) {
		return;
	}

	wlr_cursor_set_surface(DATA.compositor.cursor, event->surface, event->hotspot_x, event->hotspot_y);
}
