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

		wl_display_destroy(compositor->display);
		compositor->display = NULL;

		return -1;
	}

	compositor->renderer = wlr_renderer_autocreate(compositor->backend);
	if(!compositor->renderer) {
		fprintf(stderr, "Failed to create wlroots renderer\n");

		wlr_backend_destroy(compositor->backend);
		compositor->backend = NULL;

		wl_display_destroy(compositor->display);
		compositor->display = NULL;

		return -1;
	}

	compositor->allocator = wlr_allocator_autocreate(compositor->backend, compositor->renderer);

	if(!compositor->allocator) {
		fprintf(stderr, "Failed to create wlroots allocator\n");

		wlr_renderer_destroy(compositor->renderer);
		compositor->renderer = NULL;

		wlr_backend_destroy(compositor->backend);
		compositor->backend = NULL;

		wl_display_destroy(compositor->display);
		compositor->display = NULL;

		return -1;
	}
	
	InitEvents(&DATA.compositor, &DATA.events);

	const char *socket = wl_display_add_socket_auto(compositor->display);
	if(!socket) {
		fprintf(stderr, "Failed to create Wayland socket\n");

		wl_display_destroy(compositor->display);
		compositor->display = NULL;

		wlr_backend_destroy(compositor->backend);
		compositor->backend = NULL;

		return -1;
	}


	printf("Wayland socket: %s\n", socket);

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
