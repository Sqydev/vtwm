#ifndef VTWM_EVENTS_OUTPUT_H
#define VTWM_EVENTS_OUTPUT_H

#include <wlr/types/wlr_output.h>

typedef struct OutputEvents {
	struct wl_listener new_output;
} OutputEvents;

typedef struct VtwmOutput {
	struct wl_list link;
	struct wlr_output* wlr_output;
	struct wlr_scene_output* scene_output;
	struct wl_listener frame;
	struct wl_listener destroy;
} VtwmOutput;

void NewOutput(struct wl_listener* listener, void* data);
void OutputFrame(struct wl_listener* listener, void* data);
void DestroyOutput(struct wl_listener* listener, void* data);

#endif
