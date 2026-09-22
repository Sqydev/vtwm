#ifndef VTWM_TYPES_H
#define VTWM_TYPES_H

#include <wlr/types/wlr_output.h>
#include <wlr/render/allocator.h>

typedef struct Output {
	struct wlr_output* output;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;

	struct wl_listener frame;

	struct wlr_scene_output* sceneOutput;
} Output;

typedef struct Window {
	struct wlr_xdg_toplevel* toplevel;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;

	struct wlr_scene_node* sceneNode;
} Window;

typedef struct Layer {
	Window** windows;
	size_t windowsCount;
} Layer;

typedef enum {
	LAYER_TTY,
	LAYER_NORMAL,
	LAYER_SUPER
} LayerName;

typedef struct Workspace {
	Layer layers[3];
} Workspace;

typedef struct WorkspaceDir {
	Workspace* workspaces;
	size_t workspacesCount;
	size_t currentWorkspaceIdx;
} WorkspaceDir;

typedef struct {
	WorkspaceDir* wdir;
	size_t wdirsCount;
	size_t currentWdirIdx;
} WorkspaceManager;

#endif
