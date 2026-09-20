#ifndef VTWM_COREDATA_H
#define VTWM_COREDATA_H

#include <wlr/types/wlr_output.h>
#include <wlr/render/allocator.h>

#include "./compositor/compositor.h"
#include "./events/events.h"

typedef struct CoreData {
	Compositor compositor;

	Events events;
} CoreData;

extern CoreData DATA;

#endif
