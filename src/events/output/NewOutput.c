#include "./output.h"

#include "../../coredata.h"
#include "../../compositor/compositor.h"
#include "../../utils/utils.h"

#include <stdlib.h>

#include <wayland-util.h>
#include <wlr/util/log.h>
#include <wlr/backend/headless.h>

void NewOutput(struct wl_listener* listener, void* data) {
	OutputEvents* output_event = wl_container_of(listener, output_event, new_output);
	struct wlr_output* wlr_output = data;

	if(!wlr_output_init_render(wlr_output, COMP.allocator, COMP.renderer)) {
		wlr_log(WLR_ERROR, "Failed to initialize output rendering");
    	return;
	}

	struct wlr_output_state state; // Aka. Settings
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	// Get mode preffered mode(f.e. My monitor may preffer 165Hz and 2k resolution). If headless set fullHD
	struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
	if(mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}
	else if(wlr_output_is_headless(wlr_output)) {
		wlr_log(WLR_INFO, "Headless output. Setting mode with 1920x1080");
		wlr_output_state_set_custom_mode(&state, 1920, 1080, 0);
	}
	else {
		wlr_log(WLR_INFO, "No preferred output mode");
	}

	// Apply the settings of the output
	if(!wlr_output_commit_state(wlr_output, &state)) {
		wlr_log(WLR_ERROR, "Failed to commit output state");
		wlr_output_state_finish(&state);
		return;
	}
	// Basicly free the state mem cuz I think it's copied to somewhere else or something.
	wlr_output_state_finish(&state);

	// And flash all of the data to normal struct
	VtwmOutput* output = IFNULLPANIC(calloc(1, sizeof(*output)), 1, "Failed to allocate output\n");
	output->wlr_output = wlr_output;
	output->scene_output = IFNULLPANIC(wlr_scene_output_create(COMP.scene, wlr_output), 1, "Failed to create scene output\n");

	output->frame.notify = OutputFrame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);

	output->destroy.notify = DestroyOutput;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

	wlr_log(WLR_INFO, "Adding new output of size %dx%d", output->wlr_output->width, output->wlr_output->height);
	// Fuck around and find out :)
	wl_list_insert(&COMP.outputs, &output->link);

	struct wlr_output_layout_output* layout_output = wlr_output_layout_add_auto(COMP.output_layout, wlr_output);
	wlr_scene_output_layout_add_output(COMP.scene_output_layout, layout_output, output->scene_output);
}
