#include "../coredata.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/render/pass.h>
#include <wlr/types/wlr_output.h>

#include <stdio.h>

void Frame(struct wl_listener* listener, void* data) {
	(void)data;

	// It's really fucking fun cuz it does(we know that listener is part of Output* vtwmOutput): Give me the vtwmOutput that THIS listener is a part of;
	Output* vtwmOutput = wl_container_of(listener, vtwmOutput, frame);

	struct wlr_output_state state;
	wlr_output_state_init(&state);

	struct wlr_render_pass* pass = wlr_output_begin_render_pass(vtwmOutput->output, &state, NULL);
	if(!pass) {
		fprintf(stderr, "Failed to begin render pass\n");
		wlr_output_state_finish(&state);

		return;
	}

	struct wlr_render_rect_options rect = {
		.box = {
			.x = 0,
			.y = 0,
			.width = vtwmOutput->output->width,
			.height = vtwmOutput->output->height
		},
		.color = {
			.r = 0.0f,
			.g = 0.0f,
			.b = 0.0f,
			.a = 1.0f
		}
	};

	wlr_render_pass_add_rect(pass, &rect);

	if(!wlr_render_pass_submit(pass)) {
		fprintf(stderr, "Failed to submit render pass\n");

		wlr_output_state_finish(&state);

		return;
	}

	if(!wlr_output_commit_state(vtwmOutput->output, &state)) {
		fprintf(stderr, "Failed to commit output state\n");
	}

	wlr_output_state_finish(&state);

	wlr_output_schedule_frame(vtwmOutput->output);
}
