#ifndef VTWM_COREDATA_H
#define VTWM_COREDATA_H

#include "./compositor/compositor.h"
#include "./events/events.h"

#define COMP DATA.Compositor
#define EVENTS DATA.Events

typedef struct Coredata {
	bool debug;

	Compositor Compositor;

	Events Events;
} Coredata;

extern Coredata DATA;

#endif
