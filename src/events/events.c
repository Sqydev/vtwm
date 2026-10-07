#include "./events.h"

#include "../compositor/compositor.h"

#include <stdlib.h>
#include <wlr/types/wlr_output.h>

void InitEvents(Compositor* compositor, Events* events) {
	wl_list_init(&events->keyboardHandlers);
	events->newOutput.notify = NewOutput;

	wl_signal_add(
		&compositor->backend->events.new_output,
		&events->newOutput
	);

	events->newToplevel.notify = NewToplevel;

	wl_signal_add(
		&compositor->xdgShell->events.new_toplevel,
		&events->newToplevel
	);

	events->newInput.notify = NewInput;
	wl_signal_add(
		&compositor->backend->events.new_input,
		&events->newInput
	);

	events->pointerMotion.notify = PointerMotion;
	wl_signal_add(&compositor->cursor->events.motion, &events->pointerMotion);

	events->pointerMotionAbsolute.notify = PointerMotionAbsolute;
	wl_signal_add(&compositor->cursor->events.motion_absolute, &events->pointerMotionAbsolute);

	events->pointerButton.notify = PointerButton;
	wl_signal_add(&compositor->cursor->events.button, &events->pointerButton);

	events->pointerAxis.notify = PointerAxis;
	wl_signal_add(&compositor->cursor->events.axis, &events->pointerAxis);

	events->pointerFrame.notify = PointerFrame;
	wl_signal_add(&compositor->cursor->events.frame, &events->pointerFrame);

	events->requestSetCursor.notify = RequestSetCursor;
	wl_signal_add(&compositor->seat->events.request_set_cursor, &events->requestSetCursor);

	events->pointerFocusChange.notify = PointerFocusChange;
	wl_signal_add(&compositor->seat->pointer_state.events.focus_change, &events->pointerFocusChange);
}

void RemoveEvents(Events* events) {
	if(events->keyboardHandlers.prev != NULL) {
		KeyboardHandler* handler;
		KeyboardHandler* tmp;
		wl_list_for_each_safe(handler, tmp, &events->keyboardHandlers, link) {
			wl_list_remove(&handler->key.link);
			wl_list_remove(&handler->modifiers.link);
			wl_list_remove(&handler->destroy.link);
			wl_list_remove(&handler->link);
			free(handler);
		}
	}

	if(events->newInput.notify && events->newInput.link.prev != NULL) {
		wl_list_remove(&events->newInput.link);
	}
	
	if(events->newOutput.notify && events->newOutput.link.prev != NULL) {
		wl_list_remove(&events->newOutput.link);
	}

	if(events->newToplevel.notify && events->newToplevel.link.prev != NULL) {
		wl_list_remove(&events->newToplevel.link);
	}

	if(events->pointerMotion.notify && events->pointerMotion.link.prev != NULL) {
		wl_list_remove(&events->pointerMotion.link);
	}

	if(events->pointerMotionAbsolute.notify && events->pointerMotionAbsolute.link.prev != NULL) {
		wl_list_remove(&events->pointerMotionAbsolute.link);
	}

	if(events->pointerButton.notify && events->pointerButton.link.prev != NULL) {
		wl_list_remove(&events->pointerButton.link);
	}

	if(events->pointerAxis.notify && events->pointerAxis.link.prev != NULL) {
		wl_list_remove(&events->pointerAxis.link);
	}

	if(events->pointerFrame.notify && events->pointerFrame.link.prev != NULL) {
		wl_list_remove(&events->pointerFrame.link);
	}

	if(events->requestSetCursor.notify && events->requestSetCursor.link.prev != NULL) {
		wl_list_remove(&events->requestSetCursor.link);
	}

	if(events->pointerFocusChange.notify && events->pointerFocusChange.link.prev != NULL) {
		wl_list_remove(&events->pointerFocusChange.link);
	}
}
