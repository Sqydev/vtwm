#include "./events.h"

#include "../coredata.h"
#include "compositor/compositor.h"

#include <stdlib.h>
#include <wlr/types/wlr_keyboard.h>

static void SetKeymap(struct wlr_keyboard* keyboard) {
	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(keyboard, keymap);
	wlr_keyboard_set_repeat_info(keyboard, 25, 600);

	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
}

void NewInput(struct wl_listener* listener, void* data) {
	(void)listener;

	struct wlr_input_device* device = data;

	switch(device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD: {
			struct wlr_keyboard* keyboard = wlr_keyboard_from_input_device(device);

			SetKeymap(keyboard);

			KeyboardHandler* handler = calloc(1, sizeof(KeyboardHandler));
			if(!handler) {
				break;
			}

			handler->keyboard = keyboard;

			handler->key.notify = KeyboardKey;
			wl_signal_add(&keyboard->events.key, &handler->key);

			handler->modifiers.notify = KeyboardModifiers;
			wl_signal_add(&keyboard->events.modifiers, &handler->modifiers);

			handler->destroy.notify = KeyboardDestroy;
			wl_signal_add(&keyboard->base.events.destroy, &handler->destroy);

			wl_list_insert(&DATA.events.keyboardHandlers, &handler->link);

			wlr_seat_set_keyboard(DATA.compositor.seat, keyboard);

			break;
		}
		case WLR_INPUT_DEVICE_POINTER: {
			if(device->type == WLR_INPUT_DEVICE_POINTER) {
				wlr_cursor_attach_input_device(DATA.compositor.cursor, device);

				return;
			}
			break;
		}
		default: {
			break;
		}
	}
}
