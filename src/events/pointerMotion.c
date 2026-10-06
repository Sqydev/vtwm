#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_pointer.h>

void PointerMotion(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_pointer_motion_event* event = data;

	wlr_cursor_move(DATA.compositor.cursor, &event->pointer->base, event->delta_x, event->delta_y);

	ProcessCursorMotion(event->time_msec);
}
