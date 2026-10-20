#ifndef VTWM_EVENTS_H
#define VTWM_EVENTS_H

#include "./events/output/output.h"
#include "./events/xdg/xdg.h"

typedef struct Events {
	OutputEvents Output;
	XdgEvents Xdg;
} Events;

#endif
