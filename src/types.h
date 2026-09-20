#ifndef VTWM_TYPES_H
#define VTWM_TYPES_H

#include <wlr/types/wlr_output.h>
#include <wlr/render/allocator.h>

#include "./compositor/compositor.h"

typedef struct {
	struct wlr_output* output;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;

	struct wl_listener frame;
} Output;

typedef struct {
	struct wlr_xdg_toplevel* toplevel;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
} Window;

#endif
