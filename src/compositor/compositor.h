#ifndef VTWM_COMPOSITOR_H
#define VTWM_COMPOSITOR_H

#include "../types.h"

#include <wayland-server-core.h>

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_shm.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_scene.h>

typedef struct Compositor {
	struct wlr_compositor* compositor;
	struct wlr_xdg_shell* xdgShell;
	struct wlr_subcompositor* subcompositor;

	// Shared memory interface
	struct wlr_shm* shm;
	const char* socket;

	struct wlr_scene* scene;
	Output* output;

	struct wl_display* display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;

	// *Clipboard*
	struct wlr_data_device_manager* dataDeviceManager;
} Compositor;

int InitCompositor(Compositor* compositor);
int RunCompositor(Compositor* compositor);
void FreeCompositor(Compositor* compositor);

#endif
