/* 
* Copyright (c) 2025-present Wojciech Kaptur ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

#include "./evdev.h"

#include "./input.h"
#include "../events/input/input.h"

#include <stdlib.h>

#include <wlr/backend.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>

static uint32_t EvdevUsecToMsec(uint64_t usec) {
	return (uint32_t)(usec / UINT64_C(1000));
}

// The keyboard has the leds (caps, num lock...), it tells the hardware about them, otherwise it won't light up
static void KeyboardLedUpdate(struct wlr_keyboard* keyboard, uint32_t leds) {
	InputDevice* device = wl_container_of(keyboard, device, keyboard);
	if(!device->libinput_device) {
		return;
	}
	libinput_device_led_update(device->libinput_device, leds);
}

static const struct wlr_keyboard_impl evdev_keyboard_impl = {
	.name = "vtwm-evdev-keyboard",
	.led_update = KeyboardLedUpdate,
};

static const struct wlr_pointer_impl evdev_pointer_impl = {
	.name = "vtwm-evdev-pointer",
};

static const struct wlr_touch_impl evdev_touch_impl = {
	.name = "vtwm-evdev-touch",
};

static InputDevice* DeviceFromEvent(struct libinput_event* event) {
	struct libinput_device* libinput_device = libinput_event_get_device(event);
	if(!libinput_device) {
		return NULL;
	}
	return libinput_device_get_user_data(libinput_device);
}

static void HandleDeviceAdded(struct libinput_event* event) {
	(void)event;
}

static void HandleDeviceRemoved(struct libinput_event* event) {
	InputDevice* device = DeviceFromEvent(event);
	if(!device) {
		return;
	}

	wlr_log(WLR_INFO, "Input device removed: %s", device->path);

	// Libinput already let go of the device
	device->removed = true;
	DestroyEvdevDevice(device);

	// The same eventN may come back before inotify wakes us up
	ResyncInputSchedule();
}

static void HandleKeyboardKey(struct libinput_event* event) {
	struct libinput_event_keyboard* key_event = libinput_event_get_keyboard_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_keyboard) {
		return;
	}

	enum wl_keyboard_key_state state;
	switch(libinput_event_keyboard_get_key_state(key_event)) {
		case LIBINPUT_KEY_STATE_PRESSED: {
			state = WL_KEYBOARD_KEY_STATE_PRESSED;
			break;
		}
		case LIBINPUT_KEY_STATE_RELEASED: {
			state = WL_KEYBOARD_KEY_STATE_RELEASED;
			break;
		}
		default: {
			return;
		}
	}

	struct wlr_keyboard_key_event wlr_event = {
		.time_msec = EvdevUsecToMsec(libinput_event_keyboard_get_time_usec(key_event)),

		.keycode = libinput_event_keyboard_get_key(key_event),
		.update_state = true,
		.state = state,
	};
	wlr_keyboard_notify_key(&device->keyboard, &wlr_event);
}

static void HandlePointerMotion(struct libinput_event* event) {
	struct libinput_event_pointer* pointer_event = libinput_event_get_pointer_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_pointer) {
		return;
	}

	struct wlr_pointer_motion_event wlr_event = {
		.pointer = &device->pointer,
		.time_msec = EvdevUsecToMsec(libinput_event_pointer_get_time_usec(pointer_event)),
		.delta_x = libinput_event_pointer_get_dx(pointer_event),
		.delta_y = libinput_event_pointer_get_dy(pointer_event),
		.unaccel_dx = libinput_event_pointer_get_dx_unaccelerated(pointer_event),
		.unaccel_dy = libinput_event_pointer_get_dy_unaccelerated(pointer_event),
	};
	wl_signal_emit_mutable(&device->pointer.events.motion, &wlr_event);
	wl_signal_emit_mutable(&device->pointer.events.frame, &device->pointer);
}

static void HandlePointerMotionAbsolute(struct libinput_event* event) {
	struct libinput_event_pointer* pointer_event = libinput_event_get_pointer_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_pointer) {
		return;
	}

	struct wlr_pointer_motion_absolute_event wlr_event = {
		.pointer = &device->pointer,
		.time_msec = EvdevUsecToMsec(libinput_event_pointer_get_time_usec(pointer_event)),
		.x = libinput_event_pointer_get_absolute_x_transformed(pointer_event, 1),
		.y = libinput_event_pointer_get_absolute_y_transformed(pointer_event, 1),
	};
	wl_signal_emit_mutable(&device->pointer.events.motion_absolute, &wlr_event);
	wl_signal_emit_mutable(&device->pointer.events.frame, &device->pointer);
}

static void HandlePointerButton(struct libinput_event* event) {
	struct libinput_event_pointer* pointer_event = libinput_event_get_pointer_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_pointer) {
		return;
	}

	enum wl_pointer_button_state state;
	switch(libinput_event_pointer_get_button_state(pointer_event)) {
		case LIBINPUT_BUTTON_STATE_PRESSED: {
			state = WL_POINTER_BUTTON_STATE_PRESSED;
			break;
		}
		case LIBINPUT_BUTTON_STATE_RELEASED: {
			state = WL_POINTER_BUTTON_STATE_RELEASED;
			break;
		}
		default: {
			return;
		}
	}

	struct wlr_pointer_button_event wlr_event = {
		.pointer = &device->pointer,
		.time_msec = EvdevUsecToMsec(libinput_event_pointer_get_time_usec(pointer_event)),
		.button = libinput_event_pointer_get_button(pointer_event),
		.state = state,
	};
	wlr_pointer_notify_button(&device->pointer, &wlr_event);
	wl_signal_emit_mutable(&device->pointer.events.frame, &device->pointer);
}

static void HandlePointerAxis(struct libinput_event* event) {
	struct libinput_event_pointer* pointer_event = libinput_event_get_pointer_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_pointer) {
		return;
	}

	struct wlr_pointer_axis_event wlr_event = {
		.pointer = &device->pointer,
		.time_msec = EvdevUsecToMsec(libinput_event_pointer_get_time_usec(pointer_event)),
	};
	switch(libinput_event_pointer_get_axis_source(pointer_event)) {
		case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL: {
			wlr_event.source = WL_POINTER_AXIS_SOURCE_WHEEL;
			break;
		}
		case LIBINPUT_POINTER_AXIS_SOURCE_FINGER: {
			wlr_event.source = WL_POINTER_AXIS_SOURCE_FINGER;
			break;
		}
		case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS: {
			wlr_event.source = WL_POINTER_AXIS_SOURCE_CONTINUOUS;
			break;
		}
		case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT: {
			wlr_event.source = WL_POINTER_AXIS_SOURCE_WHEEL;
			break;
		}
		default: {
			return;
		}
	}

	const enum libinput_pointer_axis axes[] = {
		LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
		LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
	};
	for(size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); i++) {
		if(!libinput_event_pointer_has_axis(pointer_event, axes[i])) {
			continue;
		}

		if(axes[i] == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) {
			wlr_event.orientation = WL_POINTER_AXIS_VERTICAL_SCROLL;
		}
		else {
			wlr_event.orientation = WL_POINTER_AXIS_HORIZONTAL_SCROLL;
		}

		wlr_event.delta = libinput_event_pointer_get_axis_value(pointer_event, axes[i]);
		wlr_event.delta_discrete = (int32_t)(libinput_event_pointer_get_axis_value_discrete(pointer_event, axes[i]) * WLR_POINTER_AXIS_DISCRETE_STEP);
		wlr_event.relative_direction = WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL;
		if(libinput_device_config_scroll_get_natural_scroll_enabled(libinput_event_get_device(event))) {
			wlr_event.relative_direction = WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED;
		}
		wl_signal_emit_mutable(&device->pointer.events.axis, &wlr_event);
	}
	wl_signal_emit_mutable(&device->pointer.events.frame, &device->pointer);
}

static void HandleTouchDown(struct libinput_event* event) {
	struct libinput_event_touch* touch_event = libinput_event_get_touch_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_touch) {
		return;
	}

	struct wlr_touch_down_event wlr_event = {
		.touch = &device->touch,
		.time_msec = EvdevUsecToMsec(libinput_event_touch_get_time_usec(touch_event)),
		.touch_id = libinput_event_touch_get_seat_slot(touch_event),
		.x = libinput_event_touch_get_x_transformed(touch_event, 1),
		.y = libinput_event_touch_get_y_transformed(touch_event, 1),
	};
	wl_signal_emit_mutable(&device->touch.events.down, &wlr_event);
}

static void HandleTouchUp(struct libinput_event* event) {
	struct libinput_event_touch* touch_event = libinput_event_get_touch_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_touch) {
		return;
	}

	struct wlr_touch_up_event wlr_event = {
		.touch = &device->touch,
		.time_msec = EvdevUsecToMsec(libinput_event_touch_get_time_usec(touch_event)),
		.touch_id = libinput_event_touch_get_seat_slot(touch_event),
	};
	wl_signal_emit_mutable(&device->touch.events.up, &wlr_event);
}

static void HandleTouchMotion(struct libinput_event* event) {
	struct libinput_event_touch* touch_event = libinput_event_get_touch_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_touch) {
		return;
	}

	struct wlr_touch_motion_event wlr_event = {
		.touch = &device->touch,
		.time_msec = EvdevUsecToMsec(libinput_event_touch_get_time_usec(touch_event)),
		.touch_id = libinput_event_touch_get_seat_slot(touch_event),
		.x = libinput_event_touch_get_x_transformed(touch_event, 1),
		.y = libinput_event_touch_get_y_transformed(touch_event, 1),
	};
	wl_signal_emit_mutable(&device->touch.events.motion, &wlr_event);
}

static void HandleTouchCancel(struct libinput_event* event) {
	struct libinput_event_touch* touch_event = libinput_event_get_touch_event(event);
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_touch) {
		return;
	}

	struct wlr_touch_cancel_event wlr_event = {
		.touch = &device->touch,
		.time_msec = EvdevUsecToMsec(libinput_event_touch_get_time_usec(touch_event)),
		.touch_id = libinput_event_touch_get_seat_slot(touch_event),
	};
	wl_signal_emit_mutable(&device->touch.events.cancel, &wlr_event);
}

static void HandleTouchFrame(struct libinput_event* event) {
	InputDevice* device = DeviceFromEvent(event);
	if(!device || !device->has_touch) {
		return;
	}
	wl_signal_emit_mutable(&device->touch.events.frame, NULL);
}

void HandleEvdevEvent(struct libinput_event* event) {
	switch(libinput_event_get_type(event)) {
		case LIBINPUT_EVENT_DEVICE_ADDED: {
			HandleDeviceAdded(event);
			break;
		}
		case LIBINPUT_EVENT_DEVICE_REMOVED: {
			HandleDeviceRemoved(event);
			break;
		}
		case LIBINPUT_EVENT_KEYBOARD_KEY: {
			HandleKeyboardKey(event);
			break;
		}
		case LIBINPUT_EVENT_POINTER_MOTION: {
			HandlePointerMotion(event);
			break;
		}
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE: {
			HandlePointerMotionAbsolute(event);
			break;
		}
		case LIBINPUT_EVENT_POINTER_BUTTON: {
			HandlePointerButton(event);
			break;
		}
		case LIBINPUT_EVENT_POINTER_AXIS: {
			HandlePointerAxis(event);
			break;
		}
		case LIBINPUT_EVENT_TOUCH_DOWN: {
			HandleTouchDown(event);
			break;
		}
		case LIBINPUT_EVENT_TOUCH_UP: {
			HandleTouchUp(event);
			break;
		}
		case LIBINPUT_EVENT_TOUCH_MOTION: {
			HandleTouchMotion(event);
			break;
		}
		case LIBINPUT_EVENT_TOUCH_CANCEL: {
			HandleTouchCancel(event);
			break;
		}
		case LIBINPUT_EVENT_TOUCH_FRAME: {
			HandleTouchFrame(event);
			break;
		}
		default: {
			break;
		}
	}
}

bool CreateEvdevDevice(InputDevice* device) {
	struct libinput_device* libinput_device = device->libinput_device;

	const char* name = libinput_device_get_name(libinput_device);
	if(!name) {
		name = device->path;
	}

	if(libinput_device_has_capability(libinput_device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
		wlr_keyboard_init(&device->keyboard, &evdev_keyboard_impl, name);
		device->has_keyboard = true;
	}
	if(libinput_device_has_capability(libinput_device, LIBINPUT_DEVICE_CAP_POINTER)) {
		wlr_pointer_init(&device->pointer, &evdev_pointer_impl, name);
		device->has_pointer = true;
	}
	if(libinput_device_has_capability(libinput_device, LIBINPUT_DEVICE_CAP_TOUCH)) {
		wlr_touch_init(&device->touch, &evdev_touch_impl, name);
		libinput_device_get_size(libinput_device, &device->touch.width_mm, &device->touch.height_mm);
		device->has_touch = true;
	}

	if(!device->has_keyboard && !device->has_pointer && !device->has_touch) {
		wlr_log(WLR_INFO, "Input device %s has no supported capabilities", device->path);
		return false;
	}

	if(device->has_keyboard) {
		NewInputDevice(&device->keyboard.base);
	}
	if(device->has_pointer) {
		NewInputDevice(&device->pointer.base);
	}
	if(device->has_touch) {
		NewInputDevice(&device->touch.base);
	}

	return true;
}

void DestroyEvdevDevice(InputDevice* device) {
	if(!device) {
		return;
	}

	if(device->has_keyboard) {
		wlr_keyboard_finish(&device->keyboard);
		device->has_keyboard = false;
	}
	if(device->has_pointer) {
		wlr_pointer_finish(&device->pointer);
		device->has_pointer = false;
	}
	if(device->has_touch) {
		wlr_touch_finish(&device->touch);
		device->has_touch = false;
	}

	if(device->libinput_device) {
		// the DEVICE_REMOVED event that libinput sends can't find the device anymore, so it won't touch THE struct again
		libinput_device_set_user_data(device->libinput_device, NULL);

		if(!device->removed) {
			// libinput closes the fd
			libinput_path_remove_device(device->libinput_device);
			device->removed = true;
		}

		libinput_device_unref(device->libinput_device);
		device->libinput_device = NULL;
	}

	wl_list_remove(&device->link);
	free(device->path);
	free(device);
}
