#include "./compositor.h"

#include "../coredata.h"
#include "../utils/utils.h"

#include "../events/output/output.h"
#include "../events/xdg/xdg.h"

#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

void InitCompositor(void) {
	if(DATA.debug) {
		wlr_log_init(WLR_DEBUG, NULL);
	}
	else {
		wlr_log_init(WLR_INFO, NULL);
	}

	// The Wayland display is managed by libwayland. It handles accepting
	// clients from the Unix socket, manging Wayland globals, and so on.
	COMP.display = IFNULLPANIC(wl_display_create(), 1, "Failed to create wl_display\n");	

	// The backend is a wlroots feature which abstracts the underlying input and
	// output hardware. The autocreate option will choose the most suitable
	// backend based on the current environment, such as opening an X11 window
	// if an X11 server is running.
	COMP.backend = IFNULLPANIC(wlr_backend_autocreate(wl_display_get_event_loop(COMP.display), NULL), 1, "Failed to create wlr_backend\n");

	// Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The user
	// can also specify a renderer using the WLR_RENDERER env var.
	// The renderer is responsible for defining the various pixel formats it
	// supports for shared memory, this configures that for clients.
	COMP.renderer = IFNULLPANIC(wlr_renderer_autocreate(COMP.backend), 1, "Failed to create wlr_renderer\n");
	IFFALSEPANICALONE(wlr_renderer_init_wl_display(COMP.renderer, COMP.display), 1, "Failed to init wl_display\n");

	// Autocreates an allocator for us.
	// The allocator is the bridge between the renderer and the backend. It
	// handles the buffer creation, allowing wlroots to render onto the
	// screen
	COMP.allocator = IFNULLPANIC(wlr_allocator_autocreate(COMP.backend, COMP.renderer), 1, "Failed to create wlr_allocator\n");

	// This creates some hands-off wlroots interfaces. The compositor is
	// necessary for clients to allocate surfaces, the subcompositor allows to
	// assign the role of subsurfaces to surfaces and the data device manager
	// handles the clipboard. Each of these wlroots interfaces has room for you
	// to dig your fingers in and play with their behavior if you want. Note that
	// the clients cannot set the selection directly without compositor approval,
	// see the handling of the request_set_selection event below.
	wlr_compositor_create(COMP.display, 5, COMP.renderer);
	wlr_subcompositor_create(COMP.display);
	wlr_data_device_manager_create(COMP.display);

	// Creates an output layout, which a wlroots utility for working with an
	// arrangement of screens in a physical layout.
	COMP.output_layout = IFNULLPANIC(wlr_output_layout_create(COMP.display), 1, "Failed to create output layout\n");

	// Creates the scene, the render graph wlroots draws for us. Instead of
	// rendering surfaces by hand, we add them as scene nodes and let
	// wlr_scene_output handle the composition and damage tracking.
	COMP.scene = IFNULLPANIC(wlr_scene_create(), 1, "Failed to create wlr_scene\n");
	// Attaches the output layout to the scene, so each scene output keeps
	// the same position as its corresponding output layout output.
	COMP.scene_output_layout = wlr_scene_attach_output_layout(COMP.scene, COMP.output_layout);

	// Configure a listener to be notified when new outputs are available on the
	// backend.
	wl_list_init(&COMP.outputs);
	EVENTS.Output.new_output.notify = NewOutput;
	wl_signal_add(&COMP.backend->events.new_output, &EVENTS.Output.new_output);

	// Set up the xdg-shell. The xdg-shell is a Wayland protocol which is used
	// for application windows. For more detail on shells, refer tinywl's creator article:
	//
	// https://drewdevault.com/2018/07/29/Wayland-shells.html
	//
	wl_list_init(&COMP.views);
	COMP.xdg_shell = IFNULLPANIC(wlr_xdg_shell_create(COMP.display, 3), 1, "Failed to create xdg_shell\n");
	EVENTS.Xdg.new_xdg_surface.notify = NewXdgSurface;
	wl_signal_add(&COMP.xdg_shell->events.new_surface, &EVENTS.Xdg.new_xdg_surface);

	// Add a Unix socket to the Wayland display.
	COMP.socket = IFNULLPANICCONST(wl_display_add_socket_auto(COMP.display), 1, "Failed to create socket\n");
	
    // Set the WAYLAND_DISPLAY environment variable, so that clients know how to connect to our server
	setenv("WAYLAND_DISPLAY", COMP.socket, true);
}

void StartCompositor(void) {
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", DATA.Compositor.socket);

	// Start the backend. This will enumerate outputs and inputs, become the DRM
	// master, etc
	IFFALSEPANICALONE(wlr_backend_start(COMP.backend), 1, "Failed to start wl_backend");

	// Run the Wayland event loop. This does not return until you exit the
	// compositor. Starting the backend rigged up all of the necessary event
	// loop configuration to listen to libinput events, DRM events, generate
	// frame events at the refresh rate, and so on.
  	wl_display_run(COMP.display);
}

void FreeCompositor(void) {
	wl_display_destroy_clients(COMP.display);
	wl_display_destroy(COMP.display);

	if(COMP.allocator) { wlr_allocator_destroy(COMP.allocator); COMP.allocator = NULL; }
	if(COMP.renderer) { wlr_renderer_destroy(COMP.renderer); COMP.renderer = NULL; }
	if(COMP.backend) { wlr_backend_destroy(COMP.backend); COMP.backend = NULL; }
	if(COMP.display) { wl_display_destroy(COMP.display); COMP.display = NULL; }
}
