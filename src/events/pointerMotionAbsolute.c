#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_pointer.h>

void PointerMotionAbsolute(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_pointer_motion_absolute_event* event = data;

	wlr_cursor_warp_absolute(DATA.compositor.cursor, &event->pointer->base, event->x, event->y);

	ProcessCursorMotion(event->time_msec);
}
