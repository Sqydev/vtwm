#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_keyboard.h>

#include <stdlib.h>

void KeyboardDestroy(struct wl_listener* listener, void* data) {
	(void)data;
	KeyboardHandler* handler = wl_container_of(listener, handler, destroy);

	wl_list_remove(&handler->key.link);
	wl_list_remove(&handler->modifiers.link);
	wl_list_remove(&handler->destroy.link);
	wl_list_remove(&handler->link);
	free(handler);
}
