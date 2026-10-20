#ifndef VTWM_EVENTS_H
#define VTWM_EVENTS_H

#include "./output/output.h"
#include "./xdg/xdg.h"

typedef struct Events {
	OutputEvents Output;
	XdgEvents Xdg;
} Events;

#endif
