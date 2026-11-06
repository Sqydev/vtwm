#ifndef VTWM_IPC_ACTIONS_H
#define VTWM_IPC_ACTIONS_H

#include "../../config/config.h"
#include "../../events/input/input.h"

const char* HandleIpcCommand(const char* command);

// Actions
const char* ReloadConfigIpcAction(void);

#endif
