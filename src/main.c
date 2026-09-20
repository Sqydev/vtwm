#include "./coredata.h"
#include "./compositor/compositor.h"
#include "./utils/utils.h"

#include <stdlib.h>

CoreData DATA;

int main(void) {
	if(InitCompositor(&DATA.compositor) != 0) { Panic(NULL, EXIT_FAILURE); }

	RunCompositor(&DATA.compositor);

	FreeCompositor(&DATA.compositor);

	return EXIT_SUCCESS;
}
