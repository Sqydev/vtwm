#include "../coredata.h"

#include "./compositor.h"
#include "../events/events.h"

#include <stdio.h>
#include <stdlib.h>

#include <wayland-server-core.h>

int InitCompositor(Compositor* compositor) {
	compositor->display = wl_display_create();
	if(!compositor->display) {
		fprintf(stderr, "Failed to create Wayland display\n");
		return -1;
	}

	compositor->backend = wlr_backend_autocreate(wl_display_get_event_loop(compositor->display), NULL);
	if(!compositor->backend) {
		fprintf(stderr, "Failed to create wlroots backend\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->renderer = wlr_renderer_autocreate(compositor->backend);
	if(!compositor->renderer) {
		fprintf(stderr, "Failed to create wlroots renderer\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->allocator = wlr_allocator_autocreate(compositor->backend, compositor->renderer);
	if(!compositor->allocator) {
		fprintf(stderr, "Failed to create wlroots allocator\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->compositor = wlr_compositor_create(compositor->display, 5, compositor->renderer);
	if(!compositor->compositor) {
		fprintf(stderr, "Failed to create wl_compositor\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->scene = wlr_scene_create();
	if(!compositor->scene) {
		fprintf(stderr, "Failed to create scene\n");
		FreeCompositor(compositor);
	
		return -1;
	}

	compositor->subcompositor = wlr_subcompositor_create(compositor->display);
	if(!compositor->subcompositor) {
		fprintf(stderr, "Failed to create wl_subcompositor\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->shm = wlr_shm_create_with_renderer(compositor->display, 1, compositor->renderer);
	if(!compositor->shm) {
		fprintf(stderr, "Failed to create wl_shm\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->xdgShell = wlr_xdg_shell_create(compositor->display, 3);
	if(!compositor->xdgShell) {
		fprintf(stderr, "Failed to create xdg-shell\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->dataDeviceManager = wlr_data_device_manager_create(compositor->display);

	if(!compositor->dataDeviceManager) {
		fprintf(stderr, "Failed to create wl_data_device_manager\n");
		FreeCompositor(compositor);

		return -1;
	}
	
	InitEvents(&DATA.compositor, &DATA.events);

	compositor->socket = wl_display_add_socket_auto(compositor->display);
	if(!compositor->socket) {
		fprintf(stderr, "Failed to create Wayland socket\n");
		FreeCompositor(compositor);

		return -1;
	}

	printf("Wayland socket: %s\n", compositor->socket);

	return 0;
}

int RunCompositor(Compositor* compositor) {
	if(!wlr_backend_start(compositor->backend)) {
		fprintf(stderr, "Failed to start wlroots backend\n");

		return -1;
	}

	wl_display_run(compositor->display);

	return 0;
}

void FreeCompositor(Compositor* compositor) {
	if(compositor->allocator) {
		wlr_allocator_destroy(compositor->allocator);
		compositor->allocator = NULL;
	}

	if(compositor->renderer) {
		wlr_renderer_destroy(compositor->renderer);
		compositor->renderer = NULL;
	}

	if(compositor->backend) {
		wlr_backend_destroy(compositor->backend);
		compositor->backend = NULL;
	}

	if(compositor->display) {
		wl_display_destroy_clients(compositor->display);
		wl_display_destroy(compositor->display);

		compositor->display = NULL;
	}
}
