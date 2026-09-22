#include "./events.h"

#include "../coredata.h"
#include "../types.h"

#include <stdio.h>
#include <stdlib.h>

void NewToplevel(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_xdg_toplevel* toplevel = data;

	Window* window = calloc(1, sizeof(Window));
	if(!window) {
		fprintf(stderr, "Failed to allocate Window\n");
		return;
	}

	window->toplevel = toplevel;

	struct wlr_scene_tree* tree = wlr_scene_xdg_surface_create(&DATA.compositor.scene->tree, toplevel->base);
	window->sceneNode = &tree->node;
	wlr_scene_node_set_position(window->sceneNode, 0, 0);

	window->map.notify = MapWindow;
	wl_signal_add(&toplevel->base->surface->events.map, &window->map);

	window->unmap.notify = UnmapWindow;
	wl_signal_add(&toplevel->base->surface->events.unmap, &window->unmap);

	window->commit.notify = CommitWindow;
	wl_signal_add(&toplevel->base->surface->events.commit, &window->commit);

	window->destroy.notify = DestroyWindow;
	wl_signal_add(&toplevel->events.destroy, &window->destroy);

	printf("New window: %s (%s)\n", toplevel->title ? toplevel->title : "(no title)", toplevel->app_id ? toplevel->app_id : "(no app_id)");
}
