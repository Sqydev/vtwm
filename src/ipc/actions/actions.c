#include "./actions.h"

#include <string.h>

const char* HandleIpcCommand(const char* commandText) {
	if(!commandText) {
		return "unknown";
	}

	while(*commandText == ' ' || *commandText == '\t') {
		commandText++;
	}

	size_t commandLength = strlen(commandText);

	while(commandLength > 0 && (commandText[commandLength - 1] == ' ' || commandText[commandLength - 1] == '\t')) {
		commandLength--;
	}

	if(commandLength == 6 && strncmp(commandText, "reload", commandLength) == 0) {
		return ReloadConfigIpcAction();
	}

	return "unknown";
}
