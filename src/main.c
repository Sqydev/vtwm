#include "./coredata.h"
#include "./compositor/compositor.h"
#include "./utils/utils.h"

#include <stdlib.h>

CoreData DATA;

int main(void) {
	srand((unsigned)time(NULL));

	if(InitCompositor(&DATA.compositor) != 0) { Panic(EXIT_FAILURE, NULL); }

	RunCompositor(&DATA.compositor);

	FreeCompositor(&DATA.compositor);

	return EXIT_SUCCESS;
}
