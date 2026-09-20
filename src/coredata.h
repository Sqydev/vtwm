#ifndef COREDATA_VTWM_H
#define COREDATA_VTWM_H

#include <wlr/types/wlr_output.h>
#include <wlr/render/allocator.h>

#include "./compositor/compositor.h"
#include "./events/events.h"

typedef struct {
	struct wlr_output* output;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;

	struct wl_listener frame;
} Output;

typedef struct CoreData {
	Compositor compositor;

	Events events;
} CoreData;

extern CoreData DATA;

#endif
