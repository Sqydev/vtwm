#include "../xdg.h"

#include "../../../utils/utils.h"

#include <stdlib.h>

#include <wlr/util/log.h>

typedef struct VtwmPopup {
	struct wlr_xdg_popup* xdg_popup;
	struct wl_listener commit;
	struct wl_listener destroy;
} VtwmPopup;

static void CommitPopup(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmPopup* popup = wl_container_of(listener, popup, commit);

	if(popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}
}

static void DestroyPopup(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmPopup* popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->commit.link);
	wl_list_remove(&popup->destroy.link);
	free(popup);
}

void NewPopup(struct wl_listener* listener, void* data) {
	(void)listener;
	struct wlr_xdg_popup* xdg_popup = data;

	VtwmPopup* popup = IFNULLPANIC(calloc(1, sizeof(*popup)), 1, "Failed to allocate popup\n");
	popup->xdg_popup = xdg_popup;

	struct wlr_xdg_surface* parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
	if(parent != NULL && parent->data != NULL) {
		xdg_popup->base->data = wlr_scene_xdg_surface_create(parent->data, xdg_popup->base);
	}
	else {
		wlr_log(WLR_ERROR, "Xdg popup has no valid scene parent");
	}

	popup->commit.notify = CommitPopup;
	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

	popup->destroy.notify = DestroyPopup;
	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}
