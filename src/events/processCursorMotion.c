#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>

void ProcessCursorMotion(uint32_t time) {
	struct wlr_seat* seat = DATA.compositor.seat;

	double sx = 0.0;
	double sy = 0.0;

	struct wlr_scene_node* node = wlr_scene_node_at(&DATA.compositor.scene->tree.node, DATA.compositor.cursor->x, DATA.compositor.cursor->y, &sx, &sy);

	struct wlr_scene_surface* sceneSurface = NULL;
	if(node && node->type == WLR_SCENE_NODE_BUFFER) {
		sceneSurface = wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node));
	}

	if(!sceneSurface) {
		wlr_cursor_set_xcursor(DATA.compositor.cursor, DATA.compositor.cursorManager, "default");
		wlr_seat_pointer_clear_focus(seat);

		return;
	}

	wlr_seat_pointer_notify_enter(seat, sceneSurface->surface, sx, sy);
	wlr_seat_pointer_notify_motion(seat, time, sx, sy);
}
