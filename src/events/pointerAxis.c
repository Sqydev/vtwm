#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_pointer.h>

void PointerAxis(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_pointer_axis_event* event = data;

	wlr_seat_pointer_notify_axis(DATA.compositor.seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
}
