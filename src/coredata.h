#ifndef VTWM_COREDATA_H
#define VTWM_COREDATA_H

#include "./compositor/compositor.h"

#define COMP DATA.Compositor

typedef struct Coredata {
	bool debug;

	Compositor Compositor;
} Coredata;

extern Coredata DATA;

#endif
