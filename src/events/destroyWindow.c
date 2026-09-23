#include "../types.h"

#include "../coredata.h"

#include <stdlib.h>

void DestroyWindow(struct wl_listener* listener, void* data) {
	(void)data;

	Window* window = wl_container_of(listener, window, destroy);

	wl_list_remove(&window->destroy.link);
	wl_list_remove(&window->map.link);
	wl_list_remove(&window->unmap.link);
	wl_list_remove(&window->commit.link);

	WorkspaceDir* direr = &DATA.workspaceManager.wdir[window->wdirIdx];
	Workspace* workerer = &direr->workspaces[window->layer][window->workspaceIdx];

	for(size_t i = window->index; i < workerer->windowsCount - 1; i++) {
		workerer->windows[i] = workerer->windows[i + 1];
		workerer->windows[i]->index--;
	}
	wlr_scene_node_destroy(window->sceneNode);
	free(window);
	workerer->windows[--workerer->windowsCount] = NULL;
}
