#ifndef VTWM_WINDOW_MANAGER_H
#define VTWM_WINDOW_MANAGER_H

#include <stddef.h>

#include <wayland-server-core.h>

typedef enum LayerName {
	LAYER_TTY = 0,
	LAYER_NORMAL = 1,
	LAYER_SUPER = 2
} LayerName;

typedef struct Window {
	size_t ID;
	size_t index;
	LayerName layer;
	size_t workspaceIdx;
	size_t wdirIdx;

	struct wlr_xdg_toplevel* toplevel;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;

	struct wlr_scene_node* sceneNode;
} Window;

typedef struct Workspace {
	Window** windows;
	size_t windowsCount;
} Workspace;

typedef struct WorkspaceDir {
	Workspace* workspaces[3];

	size_t workspacesCount;
	size_t currentWorkspaceIdx;
} WorkspaceDir;

typedef struct WindowManager {
	LayerName currLayer;

	WorkspaceDir* wdir;

	size_t lastWindowID;

	size_t wdirsCount;
	size_t currentWdirIdx;
} WindowManager;

#endif
