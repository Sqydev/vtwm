#include "./system.h"

#include "../../coredata.h"

#include <time.h>

#include <wlr/util/log.h>

static struct timespec SubtractTimespec(struct timespec a, struct timespec b) {
	struct timespec result;

	result.tv_sec = a.tv_sec - b.tv_sec;
	result.tv_nsec = a.tv_nsec - b.tv_nsec;

	if(result.tv_nsec < 0) {
		result.tv_sec -= 1;
		result.tv_nsec += 1000000000L;
	}

	return result;
}

struct timespec GetUptime(void) {
	struct timespec now;

	if(clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
		struct timespec zero = { 0, 0 };
		return zero;
	}

	return SubtractTimespec(now, DATA.startTime);
}
