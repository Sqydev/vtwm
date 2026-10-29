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

#include "./input.h"
#include "./evdev.h"

#include "../coredata.h"
#include "../compositor/compositor.h"
#include "../utils/utils.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <wlr/backend.h>
#include <wlr/util/log.h>

// Libinput asks to open every device it knows about, so this is the only chance to keep the compositor away from the rest of /dev
static int OpenRestricted(const char* path, int flags, void* user_data) {
	(void)user_data;

	// Only the input devices, nothing else with the same major
	if(strncmp(path, INPUT_DEVICE_DIR "/", strlen(INPUT_DEVICE_DIR) + 1) != 0) {
		return -ENODEV;
	}

	// Check the thing actually opened, not the path that may have been swapped between the check and the open.
	// O_NOFOLLOW also keeps us away from the bad and evil symlinks in /dev/input
	int fd = open(path, flags | O_CLOEXEC | O_NOFOLLOW);
	if(fd < 0) {
		return -errno;
	}

	struct stat st;
	if(fstat(fd, &st) != 0) {
		int error = errno;
		close(fd);
		return -error;
	}
	// Here's the fun thing with major. SO, S_ISCHR checks if it's char file(inputtt), and major gives the major of the file, and.... yk, it's basicly just a metadata of the file and if it's 13(defoult on linux) you know 100% it's input file.
	if(!S_ISCHR(st.st_mode) || major(st.st_rdev) != INPUT_DEVICE_MAJOR) {
		close(fd);
		return -ENODEV;
	}

	return fd;
}

static void CloseRestricted(int fd, void* user_data) {
	(void)user_data;
	close(fd);
}

// Libinput has its own log, we push it into the wlroots one, so everything lands in the same place.
static void LogHandler(struct libinput* libinput, enum libinput_log_priority priority, const char* format, va_list args) {
	(void)libinput;

	enum wlr_log_importance importance = WLR_INFO;
	switch(priority) {
		case LIBINPUT_LOG_PRIORITY_DEBUG: {
			importance = WLR_DEBUG;
			break;
		}
		case LIBINPUT_LOG_PRIORITY_INFO: {
			importance = WLR_INFO;
			break;
		}
		case LIBINPUT_LOG_PRIORITY_ERROR: {
			importance = WLR_ERROR;
			break;
		}
	}

	char buffer[512];
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-nonliteral"
		int written = vsnprintf(buffer, sizeof(buffer), format, args);
	#pragma GCC diagnostic pop
	if(written < 0) {
		return;
	}
	if((size_t)written >= sizeof(buffer)) {
		buffer[sizeof(buffer) - 1] = '\0';
	}

	size_t length = strlen(buffer);
	while(length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
		buffer[--length] = '\0';
	}

	wlr_log(importance, "libinput: %s", buffer);
}

// Take a wild guess.
static const struct libinput_interface input_interface = {
	.open_restricted = OpenRestricted,
	.close_restricted = CloseRestricted,
};

// Libinput fd has something to read.
static int DispatchInput(int fd, uint32_t mask, void* data) {
	(void)fd;
	(void)data;

	if(!(mask & (WL_EVENT_READABLE | WL_EVENT_HANGUP | WL_EVENT_ERROR))) {
		return 0;
	}

	if(libinput_dispatch(INPUT.Hardware.context) != 0) {
		wlr_log(WLR_ERROR, "Failed to dispatch libinput events");
		return 0;
	}

	// The events are waiting in the queue, we take them one by one.
	struct libinput_event* event = NULL;
	while((event = libinput_get_event(INPUT.Hardware.context))) {
		HandleEvdevEvent(event);
		libinput_event_destroy(event);
	}
	return 0;
}

// YKHM, Called when something chainged in the input path.
static int InputInotify(int fd, uint32_t mask, void* data) {
	(void)mask;
	(void)data;

	char buffer[4096];
	ssize_t length = 0;
	while((length = read(fd, buffer, sizeof(buffer))) > 0) {
		// Just drain the queue
	}
	if(length < 0 && errno != EAGAIN && errno != EINTR) {
		wlr_log(WLR_ERROR, "Failed to read inotify events: %s", strerror(errno));
	}

	SyncInput();
	return 0;
}

// Adds a device node to libinput and makes the wlr devices for it
static void AddInputDevice(const char* path) {
	struct stat st;
	if(stat(path, &st) != 0) {
		wlr_log(WLR_INFO, "Input device is not available: %s: %s", path, strerror(errno));
		return;
	}

	struct libinput_device* libinput_device = libinput_path_add_device(INPUT.Hardware.context, path);
	if(!libinput_device) {
		// Libinput refuses the devices it doesn't understand, that's not fatal
		wlr_log(WLR_INFO, "libinput rejected device: %s", path);
		return;
	}

	InputDevice* device = IFNULLPANIC(calloc(1, sizeof(*device)), 1, "Failed to allocate input device\n");
	device->path = IFNULLPANIC(strdup(path), 1, "Failed to duplicate input device path\n");
	device->rdev = st.st_rdev;

	// Libinput throws the pointer away after every dispatch, so we keep our own ref to the device
	libinput_device_ref(libinput_device);
	device->libinput_device = libinput_device;

	libinput_device_set_user_data(libinput_device, device);

	wl_list_insert(&INPUT.Hardware.devices, &device->link);

	wlr_log(WLR_INFO, "Input device added: %s", path);

	if(!CreateEvdevDevice(device)) {
		// Nothing we can use, so we don't keep the device around
		wlr_log(WLR_INFO, "Input device has no supported capabilities: %s", path);
		DestroyEvdevDevice(device);
	}
}

// Makes the device list match what's in /dev/input right now.
// There's no udev to ask, so the files themselves are THE source of truth
void SyncInput(void) {
	InputDevice* device = NULL;
	InputDevice* tmp = NULL;
	wl_list_for_each_safe(device, tmp, &INPUT.Hardware.devices, link) {
		struct stat st;
		if(stat(device->path, &st) != 0) {
			wlr_log(WLR_INFO, "Input device vanished: %s", device->path);
			DestroyEvdevDevice(device);
			continue;
		}

		if(device->rdev != st.st_rdev) {
			wlr_log(WLR_INFO, "Input device was replaced: %s", device->path);
			DestroyEvdevDevice(device);
		}
	}

	DIR* dir = opendir(INPUT_DEVICE_DIR);
	if(!dir) {
		wlr_log(WLR_ERROR, "Failed to open %s: %s", INPUT_DEVICE_DIR, strerror(errno));
		return;
	}

	struct dirent* entry = NULL;
	while((entry = readdir(dir))) {
		if(strncmp(entry->d_name, "event", 5) != 0) {
			continue;
		}

		char path[PATH_MAX];
		int length = snprintf(path, sizeof(path), "%s/%s", INPUT_DEVICE_DIR, entry->d_name);
		if(length < 0 || length >= (int)sizeof(path)) {
			continue;
		}

		bool found = false;
		wl_list_for_each(device, &INPUT.Hardware.devices, link) {
			if(strcmp(device->path, path) == 0) {
				found = true;
				break;
			}
		}
		if(!found) {
			AddInputDevice(path);
		}
	}
	closedir(dir);
}

static void SyncInputIdle(void* data) {
	(void)data;
	INPUT.Hardware.reconcile_source = NULL;
	SyncInput();
}

void ResyncInputSchedule(void) {
	if(INPUT.Hardware.reconcile_source || !COMP.display) {
		return;
	}

	struct wl_event_loop* loop = wl_display_get_event_loop(COMP.display);
	INPUT.Hardware.reconcile_source = wl_event_loop_add_idle(loop, SyncInputIdle, NULL);
}

void InitInput(void) {
	if(COMP.nested) {
		return;
	}

	wl_list_init(&INPUT.Hardware.devices);
	INPUT.Hardware.inotify_fd = -1;
	INPUT.Hardware.reconcile_source = NULL;

	// Create context with like, yk, look at that struct. Like, custom handlers
	INPUT.Hardware.context = libinput_path_create_context(&input_interface, NULL);
	if(!INPUT.Hardware.context) {
		wlr_log(WLR_ERROR, "Failed to create libinput context");
		return;
	}
	libinput_log_set_handler(INPUT.Hardware.context, LogHandler);

	// Create the thing that tells vtwm that something chainged in INPUT_DEVICE_DIR
	INPUT.Hardware.inotify_fd = inotify_init1(O_NONBLOCK | O_CLOEXEC);
	if(INPUT.Hardware.inotify_fd < 0) {
		wlr_log(WLR_ERROR, "Failed to create inotify fd: %s", strerror(errno));
		DestroyInput();
		return;
	}
	if(inotify_add_watch(INPUT.Hardware.inotify_fd, INPUT_DEVICE_DIR, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MOVED_TO | IN_MOVED_FROM) < 0) {
		wlr_log(WLR_ERROR, "Failed to watch %s: %s", INPUT_DEVICE_DIR, strerror(errno));
		DestroyInput();
		return;
	}

	struct wl_event_loop* loop = wl_display_get_event_loop(COMP.display);
	INPUT.Hardware.libinput_source = wl_event_loop_add_fd(loop, libinput_get_fd(INPUT.Hardware.context), WL_EVENT_READABLE, DispatchInput, NULL);
	INPUT.Hardware.inotify_source = wl_event_loop_add_fd(loop, INPUT.Hardware.inotify_fd, WL_EVENT_READABLE, InputInotify, NULL);
	if(!INPUT.Hardware.libinput_source || !INPUT.Hardware.inotify_source) {
		wlr_log(WLR_ERROR, "Failed to add event sources");
		DestroyInput();
		return;
	}

	// From now on the devices are vtwm's, so the ones from the wlroots backend have to be ignored
	INPUT.Hardware.own_context = true;

	SyncInput();
}

void DestroyInput(void) {
	if(COMP.nested) {
		return;
	}

	if(INPUT.Hardware.reconcile_source) {
		wl_event_source_remove(INPUT.Hardware.reconcile_source);
		INPUT.Hardware.reconcile_source = NULL;
	}
	if(INPUT.Hardware.inotify_source) {
		wl_event_source_remove(INPUT.Hardware.inotify_source);
		INPUT.Hardware.inotify_source = NULL;
	}
	if(INPUT.Hardware.libinput_source) {
		wl_event_source_remove(INPUT.Hardware.libinput_source);
		INPUT.Hardware.libinput_source = NULL;
	}
	if(INPUT.Hardware.inotify_fd >= 0) {
		close(INPUT.Hardware.inotify_fd);
		INPUT.Hardware.inotify_fd = -1;
	}

	InputDevice* device = NULL;
	InputDevice* tmp = NULL;
	wl_list_for_each_safe(device, tmp, &INPUT.Hardware.devices, link) {
		DestroyEvdevDevice(device);
	}

	if(INPUT.Hardware.context) {
		libinput_unref(INPUT.Hardware.context);
		INPUT.Hardware.context = NULL;
	}
	INPUT.Hardware.own_context = false;
}
