/* 
* Copyright (c) 2025-present W.K. ( _Sqyd_ / Sqydev )
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
#include "../../wm/wdir/wdir.h"

#include <stdlib.h>

#include <wayland-util.h>
#include <wlr/util/log.h>
#include <wlr/backend/headless.h>

static ConfigOutputRule ResolveOutputRule(const char* name) {
	ConfigOutputRule result = { 0 };
	ConfigOutputRule* exact = NULL;
	ConfigOutputRule* forEach = NULL;

	for(size_t i = 0; i < CONFIG.output.ruleCount; i++) {
		if(CONFIG.output.rules[i].name == NULL) { forEach = &CONFIG.output.rules[i]; }
		else if(name && strcmp(CONFIG.output.rules[i].name, name) == 0) { exact = &CONFIG.output.rules[i]; }
	}

	if(forEach) { result = *forEach; }

	if(exact) {
		if(exact->hzType != CONFIG_OUTPUT_VALUE_UNSET) {
			result.hzType = exact->hzType;
			result.hz = exact->hz;
		}
		if(exact->resType != CONFIG_OUTPUT_VALUE_UNSET) {
			result.resType = exact->resType;
			result.resWidth = exact->resWidth;
			result.resHeight = exact->resHeight;
		}
		if(exact->wdirType != CONFIG_OUTPUT_VALUE_UNSET) {
			result.wdirType = exact->wdirType;
			result.wdir = exact->wdir;
		}
		if(exact->workspaceType != CONFIG_OUTPUT_VALUE_UNSET) {
			result.workspaceType = exact->workspaceType;
			result.workspace = exact->workspace;
		}
	}
	return result;
}

static bool ModeMatchesResolution(ConfigOutputRule* rule, struct wlr_output_mode* mode) {
	if(rule->resType != CONFIG_OUTPUT_VALUE_NUMBER) { return true; }
	return mode->width == rule->resWidth && mode->height == rule->resHeight;
}

static bool ModeMatchesRefresh(ConfigOutputRule* rule, struct wlr_output_mode* mode) {
	if(rule->hzType != CONFIG_OUTPUT_VALUE_NUMBER) { return true; }

	int target = rule->hz * 1000;
	int diff = mode->refresh - target;
	if(diff < 0) { diff = -diff; }

	return diff <= 1000;
}

static struct wlr_output_mode* SelectOutputMode(struct wlr_output* wlr_output, ConfigOutputRule* rule) {
	if(!rule) { return wlr_output_preferred_mode(wlr_output); }

	struct wlr_output_mode* best = NULL;
	struct wlr_output_mode* mode;

	wl_list_for_each(mode, &wlr_output->modes, link) {
		// I know that wlr arleady does this set of logs but 1. I arleady did that and 2. I requie more CONTEXT!
		mode->preferred ? wlr_log(WLR_DEBUG, "Checking prefered output mode with:") : wlr_log(WLR_DEBUG, "Checking output mode with:");
		wlr_log(WLR_DEBUG, "Refresh rate: %d(%dhz)", mode->refresh, mode->refresh / 1000);
		wlr_log(WLR_DEBUG, "Resolution: %dx%d", mode->width, mode->height);
		wlr_log(WLR_DEBUG, "Aspect ratio: %d", mode->picture_aspect_ratio);

		if(!ModeMatchesResolution(rule, mode)) { continue; }
		if(!ModeMatchesRefresh(rule, mode)) { continue; }

		if(!best) { best = mode; continue; }

		long area = (long)mode->width * (long)mode->height;
		long bestArea = (long)best->width * (long)best->height;

		if(rule->resType == CONFIG_OUTPUT_VALUE_MAX && area != bestArea) {
			if(area > bestArea) { best = mode; }
		}
		else if(mode->refresh > best->refresh) {
			best = mode;
		}
	}

	return best ? best : wlr_output_preferred_mode(wlr_output);
}

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

	ConfigOutputRule rule = ResolveOutputRule(wlr_output->name);

	struct wlr_output_mode* mode = SelectOutputMode(wlr_output, &rule);
	if(mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}
	else if(wlr_output_is_headless(wlr_output)) {
		int width = 1920;
		int height = 1080;
		int refresh = 0;

		if(rule.resType == CONFIG_OUTPUT_VALUE_NUMBER) {
			width = rule.resWidth;
			height = rule.resHeight;
		}
		if(rule.hzType == CONFIG_OUTPUT_VALUE_NUMBER) {
			refresh = rule.hz * 1000;
		}

		wlr_log(WLR_INFO, "Headless output. Setting mode with %dx%d", width, height);
		wlr_output_state_set_custom_mode(&state, width, height, refresh);
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

	ConfigureOutputCursor(output);
}
