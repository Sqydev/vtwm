#include "../input.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

int GetVTNumber(void) {
	int fd = open("/dev/tty", O_RDONLY | O_NOCTTY);
	if(fd < 0) {
		return -1;
	}

	struct vt_stat state;
	int tty = -1;
	if(ioctl(fd, VT_GETSTATE, &state) == 0) {
		tty = state.v_active;
	}

	close(fd);
	return tty;
}
