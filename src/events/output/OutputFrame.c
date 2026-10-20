#include "./output.h"

#include "../../compositor/compositor.h"

#include <time.h>

#include <wayland-util.h>

void OutputFrame(struct wl_listener* listener, void* data) {
	(void)data;
	VtwmOutput* output = wl_container_of(listener, output, frame);

	if(!wlr_scene_output_commit(output->scene_output, NULL)) {
		return;
	}

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(output->scene_output, &now);
}
