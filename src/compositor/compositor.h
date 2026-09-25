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
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

typedef struct Compositor {
	struct wlr_compositor* compositor;
	struct wlr_xdg_shell* xdgShell;
	struct wlr_subcompositor* subcompositor;

	// Shared memory interface
	struct wlr_shm* shm;
	const char* socket;

	struct wlr_seat* seat;
	struct wlr_keyboard* keyboard;

	struct wlr_scene* scene;
	struct wlr_scene_output_layout* sceneLayout;
	struct wlr_output_layout* outputLayout;

	struct wlr_linux_dmabuf_v1* linuxDmaBuf;

	struct wl_display* display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;

	Output* output;

	// *Clipboard*
	struct wlr_data_device_manager* dataDeviceManager;
} Compositor;

int InitCompositor(Compositor* compositor);
int RunCompositor(Compositor* compositor);
void FreeCompositor(Compositor* compositor);

#endif
