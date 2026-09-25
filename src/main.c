#include "./coredata.h"
#include "./compositor/compositor.h"
#include "./utils/utils.h"

#include <stdlib.h>

CoreData DATA;

int main(void) {
	srand((unsigned)time(NULL));

	InitPanic();

	if(InitCompositor(&DATA.compositor) != 0) { Panic(EXIT_FAILURE, NULL); }

	RunCompositor(&DATA.compositor);

	return Finish();
}
