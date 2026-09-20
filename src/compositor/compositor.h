#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <wayland-server-core.h>

#include <wlr/backend.h>
#include <wlr/render/allocator.h>

typedef struct Compositor {
	struct wl_display* display;
	struct wlr_backend* backend;
	struct wlr_renderer* renderer;
	struct wlr_allocator* allocator;
} Compositor;

int InitCompositor(Compositor* compositor);
int RunCompositor(Compositor* compositor);
void FreeCompositor(Compositor* compositor);

#endif
