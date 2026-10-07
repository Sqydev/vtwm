#include "./evdevInput.h"

#include <libinput.h>
#include <wayland-server-core.h>
#include <limits.h>

#include <wlr/backend.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVDEV_DIR "/dev/input"

struct EvdevDevice {
	struct wl_list link;
	char* path;
	struct libinput_device* libinputDevice;
	struct wlr_keyboard wlrKb;
	struct wlr_pointer wlrPtr;
	bool hasKeyboard;
	bool hasPointer;
	bool freed;
};

struct EvdevState {
	Compositor* compositor;
	struct libinput* libinput;
	struct wl_event_source* libinputSource;
	struct wl_event_source* inotifySource;
	int inotifyFd;
	struct wl_list devices;
};

static struct EvdevState gEvdev;

static int libinputOpenRestricted(const char* path, int flags, void* userdata) {
	(void)userdata;
	return open(path, flags);
}

static void libinputCloseRestricted(int fd, void* userdata) {
	(void)userdata;
	close(fd);
}

static const struct libinput_interface libinputInterface = {
	.open_restricted = libinputOpenRestricted,
	.close_restricted = libinputCloseRestricted,
};

static uint32_t evdevUsecToMsec(uint64_t usec) {
	return (uint32_t)(usec / UINT64_C(1000));
}

static struct EvdevDevice* evdevDeviceFromKeyboard(struct wlr_keyboard* kb) {
	struct EvdevDevice* dev;
	return wl_container_of(kb, dev, wlrKb);
}

static void evdevKeyboardLedUpdate(struct wlr_keyboard* kb, uint32_t leds) {
	struct EvdevDevice* dev = evdevDeviceFromKeyboard(kb);
	libinput_device_led_update(dev->libinputDevice, leds);
}

static const struct wlr_keyboard_impl evdevKeyboardImpl = {
	.name = "vtwm-evdev-keyboard",
	.led_update = evdevKeyboardLedUpdate,
};

static const struct wlr_pointer_impl evdevPointerImpl = {
	.name = "vtwm-evdev-pointer",
};

static void evdevAdvertise(struct EvdevDevice* dev) {
	if(dev->hasKeyboard) {
		wl_signal_emit(&gEvdev.compositor->backend->events.new_input, &dev->wlrKb.base);
	}
	if(dev->hasPointer) {
		wl_signal_emit(&gEvdev.compositor->backend->events.new_input, &dev->wlrPtr.base);
	}
}

static void evdevDeviceFree(struct EvdevDevice* dev) {
	if(dev->freed) {
		return;
	}
	dev->freed = true;

	if(dev->hasKeyboard) {
		wlr_keyboard_finish(&dev->wlrKb);
	}
	if(dev->hasPointer) {
		wlr_pointer_finish(&dev->wlrPtr);
	}
	if(dev->libinputDevice) {
		libinput_device_set_user_data(dev->libinputDevice, NULL);
		libinput_device_unref(dev->libinputDevice);
	}
	wl_list_remove(&dev->link);
	free(dev->path);
	free(dev);
}

static void evdevHandleDeviceAdded(struct libinput_event* event) {
	struct libinput_device* ld = libinput_event_get_device(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(ld);
	if(!dev) {
		return;
	}

	const char* name = libinput_device_get_name(ld);
	if(libinput_device_has_capability(ld, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
		wlr_keyboard_init(&dev->wlrKb, &evdevKeyboardImpl, name);
		dev->hasKeyboard = true;
	}
	if(libinput_device_has_capability(ld, LIBINPUT_DEVICE_CAP_POINTER)) {
		wlr_pointer_init(&dev->wlrPtr, &evdevPointerImpl, name);
		dev->hasPointer = true;
	}

	if(dev->hasKeyboard || dev->hasPointer) {
		evdevAdvertise(dev);
	}
}

static void evdevHandleDeviceRemoved(struct libinput_event* event) {
	struct libinput_device* ld = libinput_event_get_device(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(ld);
	if(!dev) {
		return;
	}
	evdevDeviceFree(dev);
}

static void evdevHandleKeyboardKey(struct libinput_event* event) {
	struct libinput_event_keyboard* ke = libinput_event_get_keyboard_event(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(libinput_event_get_device(event));
	if(!dev || !dev->hasKeyboard) {
		return;
	}

	enum libinput_key_state state = libinput_event_keyboard_get_key_state(ke);
	enum wl_keyboard_key_state wlrState;
	if(state == LIBINPUT_KEY_STATE_PRESSED) {
		wlrState = WL_KEYBOARD_KEY_STATE_PRESSED;
	} else if(state == LIBINPUT_KEY_STATE_RELEASED) {
		wlrState = WL_KEYBOARD_KEY_STATE_RELEASED;
	} else {
		return;
	}

	struct wlr_keyboard_key_event wev = {
		.time_msec = evdevUsecToMsec(libinput_event_keyboard_get_time_usec(ke)),
		.keycode = libinput_event_keyboard_get_key(ke),
		.update_state = true,
		.state = wlrState,
	};
	wlr_keyboard_notify_key(&dev->wlrKb, &wev);
}

static void evdevHandlePointerMotion(struct libinput_event* event) {
	struct libinput_event_pointer* pe = libinput_event_get_pointer_event(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(libinput_event_get_device(event));
	if(!dev || !dev->hasPointer) {
		return;
	}

	struct wlr_pointer_motion_event wev = {
		.pointer = &dev->wlrPtr,
		.time_msec = evdevUsecToMsec(libinput_event_pointer_get_time_usec(pe)),
		.delta_x = libinput_event_pointer_get_dx(pe),
		.delta_y = libinput_event_pointer_get_dy(pe),
		.unaccel_dx = libinput_event_pointer_get_dx_unaccelerated(pe),
		.unaccel_dy = libinput_event_pointer_get_dy_unaccelerated(pe),
	};
	wl_signal_emit(&dev->wlrPtr.events.motion, &wev);
	wl_signal_emit(&dev->wlrPtr.events.frame, &dev->wlrPtr);
}

static void evdevHandlePointerMotionAbsolute(struct libinput_event* event) {
	struct libinput_event_pointer* pe = libinput_event_get_pointer_event(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(libinput_event_get_device(event));
	if(!dev || !dev->hasPointer) {
		return;
	}

	struct wlr_pointer_motion_absolute_event wev = {
		.pointer = &dev->wlrPtr,
		.time_msec = evdevUsecToMsec(libinput_event_pointer_get_time_usec(pe)),
		.x = libinput_event_pointer_get_absolute_x_transformed(pe, 1),
		.y = libinput_event_pointer_get_absolute_y_transformed(pe, 1),
	};
	wl_signal_emit(&dev->wlrPtr.events.motion_absolute, &wev);
	wl_signal_emit(&dev->wlrPtr.events.frame, &dev->wlrPtr);
}

static void evdevHandlePointerButton(struct libinput_event* event) {
	struct libinput_event_pointer* pe = libinput_event_get_pointer_event(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(libinput_event_get_device(event));
	if(!dev || !dev->hasPointer) {
		return;
	}

	enum libinput_button_state state = libinput_event_pointer_get_button_state(pe);
	enum wl_pointer_button_state wlrState;
	if(state == LIBINPUT_BUTTON_STATE_PRESSED) {
		wlrState = WL_POINTER_BUTTON_STATE_PRESSED;
	} else if(state == LIBINPUT_BUTTON_STATE_RELEASED) {
		wlrState = WL_POINTER_BUTTON_STATE_RELEASED;
	} else {
		return;
	}

	struct wlr_pointer_button_event wev = {
		.pointer = &dev->wlrPtr,
		.time_msec = evdevUsecToMsec(libinput_event_pointer_get_time_usec(pe)),
		.button = libinput_event_pointer_get_button(pe),
		.state = wlrState,
	};
	wlr_pointer_notify_button(&dev->wlrPtr, &wev);
	wl_signal_emit(&dev->wlrPtr.events.frame, &dev->wlrPtr);
}

static bool evdevAxisSourceToWlr(enum libinput_pointer_axis_source source,
	enum wl_pointer_axis_source* out) {
	switch(source) {
		case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
			*out = WL_POINTER_AXIS_SOURCE_WHEEL;
			return true;
		case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
			*out = WL_POINTER_AXIS_SOURCE_FINGER;
			return true;
		case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
			*out = WL_POINTER_AXIS_SOURCE_CONTINUOUS;
			return true;
		case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT:
			*out = WL_POINTER_AXIS_SOURCE_WHEEL_TILT;
			return true;
	}
	return false;
}

static void evdevHandlePointerAxis(struct libinput_event* event) {
	struct libinput_event_pointer* pe = libinput_event_get_pointer_event(event);
	struct EvdevDevice* dev = libinput_device_get_user_data(libinput_event_get_device(event));
	if(!dev || !dev->hasPointer) {
		return;
	}

	struct wlr_pointer_axis_event wev = {
		.pointer = &dev->wlrPtr,
		.time_msec = evdevUsecToMsec(libinput_event_pointer_get_time_usec(pe)),
	};
	if(!evdevAxisSourceToWlr(libinput_event_pointer_get_axis_source(pe), &wev.source)) {
		return;
	}

	const enum libinput_pointer_axis axes[] = {
		LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
		LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
	};
	for(size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) {
		if(!libinput_event_pointer_has_axis(pe, axes[i])) {
			continue;
		}

		switch(axes[i]) {
			case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
				wev.orientation = WL_POINTER_AXIS_VERTICAL_SCROLL;
				break;
			case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
				wev.orientation = WL_POINTER_AXIS_HORIZONTAL_SCROLL;
				break;
		}
		wev.delta = libinput_event_pointer_get_axis_value(pe, axes[i]);
		wev.delta_discrete =
			libinput_event_pointer_get_axis_value_discrete(pe, axes[i]) * WLR_POINTER_AXIS_DISCRETE_STEP;
		wev.relative_direction = WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL;
		if(libinput_device_config_scroll_get_natural_scroll_enabled(libinput_event_get_device(event))) {
			wev.relative_direction = WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED;
		}
		wl_signal_emit(&dev->wlrPtr.events.axis, &wev);
	}
	wl_signal_emit(&dev->wlrPtr.events.frame, &dev->wlrPtr);
}

static void evdevHandleEvent(struct libinput_event* event) {
	switch(libinput_event_get_type(event)) {
		case LIBINPUT_EVENT_DEVICE_ADDED:
			evdevHandleDeviceAdded(event);
			break;
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			evdevHandleDeviceRemoved(event);
			break;
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			evdevHandleKeyboardKey(event);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION:
			evdevHandlePointerMotion(event);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
			evdevHandlePointerMotionAbsolute(event);
			break;
		case LIBINPUT_EVENT_POINTER_BUTTON:
			evdevHandlePointerButton(event);
			break;
		case LIBINPUT_EVENT_POINTER_AXIS:
			evdevHandlePointerAxis(event);
			break;
		default:
			break;
	}
}

static int evdevLibinputCb(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)data;

	if(mask & (WL_EVENT_READABLE | WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
		if(libinput_dispatch(gEvdev.libinput) != 0) {
			return 0;
		}
		struct libinput_event* event;
		while((event = libinput_get_event(gEvdev.libinput))) {
			evdevHandleEvent(event);
			libinput_event_destroy(event);
		}
	}
	return 0;
}

static void evdevReconcile(void);

static int evdevInotifyCb(int fd, uint32_t mask, void* data) {
	(void)mask;
	(void)data;

	char buf[4096];
	while(read(fd, buf, sizeof(buf)) > 0) {
	}
	evdevReconcile();
	return 0;
}

static void evdevAddDevice(const char* path) {
	struct EvdevDevice* dev = calloc(1, sizeof(*dev));
	if(!dev) {
		return;
	}
	dev->path = strdup(path);
	if(!dev->path) {
		free(dev);
		return;
	}
	wl_list_insert(&gEvdev.devices, &dev->link);

	dev->libinputDevice = libinput_path_add_device(gEvdev.libinput, path);
	if(!dev->libinputDevice) {
		evdevDeviceFree(dev);
		return;
	}
	libinput_device_set_user_data(dev->libinputDevice, dev);
}

static void evdevReconcile(void) {
	struct EvdevDevice* dev;
	struct EvdevDevice* tmp;

	wl_list_for_each_safe(dev, tmp, &gEvdev.devices, link) {
		if(access(dev->path, F_OK) == 0) {
			continue;
		}
		if(dev->libinputDevice) {
			libinput_device_set_user_data(dev->libinputDevice, NULL);
			libinput_path_remove_device(dev->libinputDevice);
			dev->libinputDevice = NULL;
		}
		evdevDeviceFree(dev);
	}

	DIR* dir = opendir(EVDEV_DIR);
	if(!dir) {
		return;
	}

	struct dirent* ent;
	while((ent = readdir(dir))) {
		if(ent->d_name[0] == '.') {
			continue;
		}
		if(strncmp(ent->d_name, "event", 5) != 0) {
			continue;
		}

		char path[NAME_MAX + sizeof(EVDEV_DIR) + 4];
		snprintf(path, sizeof(path), "%s/%s", EVDEV_DIR, ent->d_name);

		bool found = false;
		wl_list_for_each(dev, &gEvdev.devices, link) {
			if(strcmp(dev->path, path) == 0) {
				found = true;
				break;
			}
		}
		if(!found) {
			evdevAddDevice(path);
		}
	}
	closedir(dir);
}

int InitEvdevInput(Compositor* compositor) {
	if(getenv("WAYLAND_DISPLAY") || getenv("_WAYLAND_DISPLAY")) {
		return 0;
	}

	gEvdev.compositor = compositor;
	wl_list_init(&gEvdev.devices);

	gEvdev.libinput = libinput_path_create_context(&libinputInterface, NULL);
	if(!gEvdev.libinput) {
		return -1;
	}

	gEvdev.inotifyFd = -1;
	gEvdev.inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if(gEvdev.inotifyFd < 0) {
		DestroyEvdevInput();
		return -1;
	}
	inotify_add_watch(gEvdev.inotifyFd, EVDEV_DIR,
		IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);

	struct wl_event_loop* loop = wl_display_get_event_loop(compositor->display);
	gEvdev.libinputSource = wl_event_loop_add_fd(loop,
		libinput_get_fd(gEvdev.libinput), WL_EVENT_READABLE, evdevLibinputCb, NULL);
	gEvdev.inotifySource = wl_event_loop_add_fd(loop,
		gEvdev.inotifyFd, WL_EVENT_READABLE, evdevInotifyCb, NULL);
	if(!gEvdev.libinputSource || !gEvdev.inotifySource) {
		DestroyEvdevInput();
		return -1;
	}

	evdevReconcile();
	return 0;
}

void DestroyEvdevInput(void) {
	if(!gEvdev.libinput) {
		return;
	}

	if(gEvdev.inotifySource) {
		wl_event_source_remove(gEvdev.inotifySource);
		gEvdev.inotifySource = NULL;
	}
	if(gEvdev.libinputSource) {
		wl_event_source_remove(gEvdev.libinputSource);
		gEvdev.libinputSource = NULL;
	}
	if(gEvdev.inotifyFd >= 0) {
		close(gEvdev.inotifyFd);
		gEvdev.inotifyFd = -1;
	}

	struct EvdevDevice* dev;
	struct EvdevDevice* tmp;
	wl_list_for_each_safe(dev, tmp, &gEvdev.devices, link) {
		evdevDeviceFree(dev);
	}

	libinput_unref(gEvdev.libinput);
	gEvdev.libinput = NULL;
	gEvdev.compositor = NULL;
}
