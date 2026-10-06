#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_seat.h>

void PointerFocusChange(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_seat_pointer_focus_change_event* event = data;

	if(event->new_surface == NULL) {
		wlr_cursor_set_xcursor(DATA.compositor.cursor, DATA.compositor.cursorManager, "default");
	}
}
