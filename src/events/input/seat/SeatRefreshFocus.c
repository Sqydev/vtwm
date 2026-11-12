#include "../input.h"

#include "../../../coredata.h"
#include "../../../utils/utils.h"

void SeatRefreshFocus(VtwmSeat* seat) {
	if(!seat || !INPUT.cursor) {
		return;
	}

	struct timespec now = GetUptime();
	uint32_t time = (uint32_t)(now.tv_sec * 1000 + now.tv_nsec / 1000000);

	ProcessCursorMotion(seat, time);
}
