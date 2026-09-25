#include "./events.h"

#include "../compositor/compositor.h"

#include <wlr/types/wlr_output.h>

void InitEvents(Compositor* compositor, Events* events) {
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
}

void RemoveEvents(Events* events) {
	if(events->newInput.notify && events->newInput.link.prev != NULL) {
		wl_list_remove(&events->newInput.link);
	}
	
	if(events->newOutput.notify && events->newOutput.link.prev != NULL) {
		wl_list_remove(&events->newOutput.link);
	}

	if(events->newToplevel.notify && events->newToplevel.link.prev != NULL) {
		wl_list_remove(&events->newToplevel.link);
	}
}
