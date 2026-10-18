#ifndef VTWM_COMPOSITOR_H
#define VTWM_COMPOSITOR_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

typedef struct Compositor {
	struct wl_display* display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;
	struct wlr_scene* scene;

	struct wlr_xdg_shell* xdg_shell;
	struct wl_listener new_xdg_surface;
	struct wl_list views;

	struct wlr_output_layout *output_layout;
	struct wl_list outputs;
} Compositor; 

void InitCompositor(void);
void FreeCompositor(void);

#endif
