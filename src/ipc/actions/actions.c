#include "./actions.h"

#include <stdlib.h>

#include <wlr/util/log.h>

const char* HandleIpcCommand(const char* command) {
	if(!command) {
		return "unknown";
	}

	while(*command == ' ' || *command == '\t') {
		command++;
	}

	size_t length = strlen(command);

	while(length > 0 && (command[length - 1] == ' ' || command[length - 1] == '\t')) {
		length--;
	}

	if(length == 6 && strncmp(command, "reload", length) == 0) {
		return ReloadConfigIpcAction();
	}

	return "unknown";
}
