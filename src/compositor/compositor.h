#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <wayland-server-core.h>

typedef struct compositor {
	struct wl_display* display;
} compositor;

int InitCompositor(compositor* Compositor);
int RunCompositor(compositor* Compositor);
void FreeCompositor(compositor* Compositor);

#endif
