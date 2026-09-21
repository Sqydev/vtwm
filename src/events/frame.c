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

	if(!wlr_scene_output_commit(vtwmOutput->sceneOutput, NULL)) {
		fprintf(stderr, "Failed to commit scene output\n");
	}

	wlr_output_schedule_frame(vtwmOutput->output);
}
