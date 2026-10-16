#include "./coredata.h"

#include "./utils/utils.h"
#include "./compositor/compositor.h"

#include <stddef.h>
#include <wlr/util/log.h>

Coredata DATA;

int main(void) {
	InitPanic();

	// When you make propper args handling make it be sensetve to debug flag and not compiler define.
	#ifndef NDEBUG
		wlr_log_init(WLR_DEBUG, NULL);
	#elif
		wlr_log_init(WLR_INFO, NULL);
	#endif

	InitCompositor();

	FreeCompositor();

	return Finish();
}
