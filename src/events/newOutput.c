#include "../coredata.h"

#include "../types.h"

#include <wlr/types/wlr_output.h>

#include <stdio.h>
#include <stdlib.h>

void NewOutput(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_output* output = data;

	printf("New output: %s (%dx%d)\n", output->name, output->width, output->height);

	// Here we are just copying output to vtwmOutput so like. It's preety :)
	Output* vtwmOutput = calloc(1, sizeof(Output));
	if(!vtwmOutput) {
		fprintf(stderr, "Failed to allocate vtwmOutput\n");

		return;
	}

	DATA.compositor.output = vtwmOutput;

	vtwmOutput->output = output;
	vtwmOutput->renderer = DATA.compositor.renderer;
	vtwmOutput->allocator = DATA.compositor.allocator;

	vtwmOutput->sceneOutput = wlr_scene_output_create(DATA.compositor.scene, output);
	if(!vtwmOutput->sceneOutput) {
		fprintf(stderr, "Failed to create scene output\n");
		free(vtwmOutput);
		return;
	}
	wlr_scene_rect_create(&DATA.compositor.scene->tree, output->width, output->height, (float[]){0.0f, 0.0f, 0.0f, 1.0f});

	if(!wlr_output_init_render(vtwmOutput->output, vtwmOutput->allocator, vtwmOutput->renderer)) {
		fprintf(stderr, "Failed to initialize output rendering\n");

		free(vtwmOutput);

		return;
	}

	struct wlr_output_state state;
	wlr_output_state_init(&state);

	if(!wl_list_empty(&output->modes)) {
		wlr_output_state_set_mode(&state, wlr_output_preferred_mode(output));
	}

	wlr_output_state_set_enabled(&state, true);
	wlr_output_commit_state(output, &state);
	wlr_output_state_finish(&state);

	vtwmOutput->frame.notify = Frame;

	wl_signal_add(&vtwmOutput->output->events.frame, &vtwmOutput->frame);
}
