#include "./output.h"

#include <stdlib.h>

#include <wayland-util.h>

void DestroyOutput(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmOutput* output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->link);
	free(output);
}
