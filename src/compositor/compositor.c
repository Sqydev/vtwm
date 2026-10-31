/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

#include "./compositor.h"

#include "../coredata.h"
#include "../utils/utils.h"

#include "../events/output/output.h"
#include "../events/xdg/xdg.h"
#include "../events/input/input.h"

#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

void InitCompositor(void) {
	COMP.nested = getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET") || getenv("DISPLAY");
	if(!COMP.nested && !getenv("WLR_BACKENDS")) {
		setenv("WLR_BACKENDS", "drm", 1);
	}

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

	// The seats live in their own list, so we can have more than one of them
	wl_list_init(&INPUT.seats);
	INPUT.active_seat = NULL;
	// Seat0 is the primary seat and the compositor makes it on startup, so the counter starts on 1
	INPUT.seat_counter = 1;
	INPUT.cursor = NULL;
	// One xcursor manager for all the seats, they only share the images
	INPUT.cursor_manager = IFNULLPANIC(wlr_xcursor_manager_create(NULL, 24), 1, "Failed to create xcursor manager\n");

	// The shared cursor has to exist before the seats, they use it for pointer and touch
	InitCursor();

	// Create main seat.
	// It has to be before InitInput, because the devices that are already plugged in want a seat to go to
	CreateSeat("seat0");

	EVENTS.Input.new_input.notify = NewInput;
	wl_signal_add(&COMP.backend->events.new_input, &EVENTS.Input.new_input);

	// Init custom input system
	InitInput();

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
	EVENTS.Xdg.new_toplevel.notify = NewToplevel;
	wl_signal_add(&COMP.xdg_shell->events.new_toplevel, &EVENTS.Xdg.new_toplevel);
	EVENTS.Xdg.new_popup.notify = NewPopup;
	wl_signal_add(&COMP.xdg_shell->events.new_popup, &EVENTS.Xdg.new_popup);

	// Add a Unix socket to the Wayland display.
	COMP.socket = IFNULLPANICCONST(wl_display_add_socket_auto(COMP.display), 1, "Failed to create socket\n");
	
	// Set the WAYLAND_DISPLAY environment variable, so that clients know how to connect to our server
	setenv("WAYLAND_DISPLAY", COMP.socket, true);
}

void StartCompositor(void) {
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", DATA.Compositor.socket);

	// Start the backend. This will enumerate outputs and inputs, become the DRM
	// master, etc
	IFFALSEPANICALONE(wlr_backend_start(COMP.backend), 1, "Failed to start wl_backend\n");

	// Run the Wayland event loop. This does not return until you exit the
	// compositor. Starting the backend rigged up all of the necessary event
	// loop configuration to listen to libinput events, DRM events, generate
	// frame events at the refresh rate, and so on.
  	wl_display_run(COMP.display);
}

void FreeCompositor(void) {
	// the backend asserts that nobody listens to it anymore when it goes,
	// so the global listeners leave before the teardown starts
	wl_list_remove(&EVENTS.Input.new_input.link);
	wl_list_remove(&EVENTS.Output.new_output.link);
	wl_list_remove(&EVENTS.Xdg.new_toplevel.link);
	wl_list_remove(&EVENTS.Xdg.new_popup.link);

	// the input goes first, because the seats hold the devices
	DestroyInput();

	// every seat has to die before the display, they are listening on it
	VtwmSeat* seat = NULL;
	VtwmSeat* seat_tmp = NULL;
	wl_list_for_each_safe(seat, seat_tmp, &INPUT.seats, link) {
		DestroySeat(seat);
	}

	// the shared cursor holds listeners and points at the output layout,
	// so it has to go before the layout
	DestroyCursor();

	// the cursor images belong to the manager, so it goes with it
	if(INPUT.cursor_manager) {
		wlr_xcursor_manager_destroy(INPUT.cursor_manager);
		INPUT.cursor_manager = NULL;
	}

	wl_display_destroy_clients(COMP.display);

	if(COMP.output_layout) { wlr_output_layout_destroy(COMP.output_layout); COMP.output_layout = NULL; }
	if(COMP.scene_output_layout) { COMP.scene_output_layout = NULL; }
	if(COMP.scene) { wlr_scene_node_destroy(&COMP.scene->tree.node); COMP.scene = NULL; }
	if(COMP.allocator) { wlr_allocator_destroy(COMP.allocator); COMP.allocator = NULL; }
	if(COMP.renderer) { wlr_renderer_destroy(COMP.renderer); COMP.renderer = NULL; }
	if(COMP.backend) { wlr_backend_destroy(COMP.backend); COMP.backend = NULL; }
	wl_display_destroy(COMP.display);
	COMP.display = NULL;
}
