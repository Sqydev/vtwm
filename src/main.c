#include "./coredata.h"

#include "./utils/utils.h"
#include "compositor/compositor.h"

#include <stddef.h>

Coredata DATA;

int main(void) {
	InitPanic();

	InitCompositor();

	FreeCompositor();

	return Finish();
}
