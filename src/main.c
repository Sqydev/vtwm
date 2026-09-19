#include "./compositor/compositor.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	compositor Compositor = {0};

	if(InitCompositor(&Compositor) != 0) { return EXIT_FAILURE; }

	printf("Compositor started\n");

	RunCompositor(&Compositor);

	FreeCompositor(&Compositor);

	return EXIT_SUCCESS;
}
