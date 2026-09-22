#include "../coredata.h"
#include "../types.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/render/pass.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>

#include <stdio.h>

void Frame(struct wl_listener* listener, void* data) {
	(void)data;

	Output* vtwmOutput = wl_container_of(listener, vtwmOutput, frame);

	struct wlr_scene_output* sceneOutput = wlr_scene_get_scene_output(DATA.compositor.scene, vtwmOutput->output);
	if(!sceneOutput) {
		return;
	}

	if(!wlr_scene_output_commit(sceneOutput, NULL)) {
		fprintf(stderr, "Failed to commit scene output\n");
		return;
	}

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(sceneOutput, &now);
}
