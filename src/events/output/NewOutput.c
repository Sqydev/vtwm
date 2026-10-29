/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

#include "./output.h"

#include "../../coredata.h"
#include "../../compositor/compositor.h"
#include "../../utils/utils.h"

#include <stdlib.h>

#include <wayland-util.h>
#include <wlr/util/log.h>
#include <wlr/backend/headless.h>

void NewOutput(struct wl_listener* listener, void* data) {
	(void)listener;
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

	output->request_state.notify = RequestState;
	wl_signal_add(&wlr_output->events.request_state, &output->request_state);

	output->destroy.notify = DestroyOutput;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

	wlr_log(WLR_INFO, "Adding new output of size %dx%d", output->wlr_output->width, output->wlr_output->height);
	// Fuck around and find out :)
	wl_list_insert(&COMP.outputs, &output->link);

	struct wlr_output_layout_output* layout_output = wlr_output_layout_add_auto(COMP.output_layout, wlr_output);
	if(!layout_output) {
		wlr_log(WLR_ERROR, "Failed to add output to output layout");
		wl_list_remove(&output->frame.link);
		wl_list_remove(&output->request_state.link);
		wl_list_remove(&output->destroy.link);
		wl_list_remove(&output->link);
		if(output->scene_output) {
			wlr_scene_output_destroy(output->scene_output);
			output->scene_output = NULL;
		}
		free(output);
		return;
	}
	wlr_scene_output_layout_add_output(COMP.scene_output_layout, layout_output, output->scene_output);
}
