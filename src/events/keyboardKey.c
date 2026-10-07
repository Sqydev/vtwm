#include "./events.h"

#include "../coredata.h"
#include "../utils/utils.h"

#include <wlr/types/wlr_keyboard.h>

#include <unistd.h>

void KeyboardKey(struct wl_listener* listener, void* data) {
	KeyboardHandler* handler = wl_container_of(listener, handler, key);
	struct wlr_keyboard* keyboard = handler->keyboard;
	struct wlr_keyboard_key_event* event = data;

	uint32_t keycode = event->keycode + 8;

	const xkb_keysym_t* syms = NULL;
	int nsyms = 0;
	if(keyboard->xkb_state) {
		nsyms = xkb_state_key_get_syms(keyboard->xkb_state, keycode, &syms);
	}
	uint32_t mods = wlr_keyboard_get_modifiers(keyboard);

	bool handled = false;

	if(event->state == WL_KEYBOARD_KEY_STATE_PRESSED && nsyms > 0) {
		if((mods & WLR_MODIFIER_ALT) && syms[0] == XKB_KEY_Tab) {
			pid_t pid = fork();

			if(pid == 0) {
				execlp("alacritty", "alacritty", NULL);
				_exit(1);
			}
			handled = true;
		}
		if((mods & WLR_MODIFIER_ALT) && syms[0] == XKB_KEY_Escape) {
			Close(0);
			handled = true;
		}
	}

	if(!handled) {
		wlr_seat_keyboard_notify_key(DATA.compositor.seat, event->time_msec, event->keycode, event->state);
	}
}
