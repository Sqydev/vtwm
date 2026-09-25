#include "./events.h"

#include "../coredata.h"

#include <wlr/types/wlr_keyboard.h>

void KeyboardDestroy(struct wl_listener* listener, void* data) {
    (void)data;
	(void)listener;

    wl_list_remove(&DATA.events.keyboardKey.link);
    wl_list_remove(&DATA.events.keyboardModifiers.link);
    wl_list_remove(&DATA.events.keyboardDestroy.link);
}
