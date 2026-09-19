#include "./compositor.h"

#include <stdio.h>
#include <stdlib.h>

int InitCompositor(compositor* Compositor) {
	Compositor->display = wl_display_create();
	if(!Compositor->display) {
		fprintf(stderr, "Failed to create Wayland display\n");
		return -1;
	}

	const char *socket = wl_display_add_socket_auto(Compositor->display);
	if (!socket) {
		fprintf(stderr, "Failed to create Wayland socket\n");
		wl_display_destroy(Compositor->display);
		Compositor->display = NULL;
		return -1;
	}

	printf("Wayland socket: %s\n", socket);

	return 0;
}

int RunCompositor(compositor* Compositor) {
	wl_display_run(Compositor->display);

	return 0;
}

void FreeCompositor(compositor* Compositor) {
	if(!Compositor->display) { return; }

	wl_display_destroy_clients(Compositor->display);

	wl_display_destroy(Compositor->display);

	Compositor->display = NULL;
}
