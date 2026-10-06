#include "./events.h"

#include "../coredata.h"

void PointerFrame(struct wl_listener* listener, void* data) {
	(void)listener;
	(void)data;

	wlr_seat_pointer_notify_frame(DATA.compositor.seat);
}
