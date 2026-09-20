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
}
