#include "./events.h"

#include "../coredata.h"

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
    if(device->type != WLR_INPUT_DEVICE_KEYBOARD) { return; }

    struct wlr_keyboard* keyboard = wlr_keyboard_from_input_device(device);
    DATA.compositor.keyboard = keyboard;

    SetKeymap(keyboard);

    DATA.events.keyboardKey.notify = KeyboardKey;
    wl_signal_add(&keyboard->events.key, &DATA.events.keyboardKey);

    DATA.events.keyboardModifiers.notify = KeyboardModifiers;
    wl_signal_add(&keyboard->events.modifiers, &DATA.events.keyboardModifiers);

    DATA.events.keyboardDestroy.notify = KeyboardDestroy;
    wl_signal_add(&keyboard->base.events.destroy, &DATA.events.keyboardDestroy);

    wlr_seat_set_keyboard(DATA.compositor.seat, keyboard);
}
