#ifndef VTWM_EVDEV_INPUT_H
#define VTWM_EVDEV_INPUT_H

#include "./compositor.h"

int InitEvdevInput(struct Compositor* compositor);
void DestroyEvdevInput(void);

#endif
