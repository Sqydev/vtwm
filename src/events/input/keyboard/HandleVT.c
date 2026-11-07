#include "../input.h"

#include "../../../coredata.h"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <wlr/backend/session.h>
#include <wlr/util/log.h>

bool HandleVT(xkb_keysym_t sym) {
	if(sym < XKB_KEY_XF86Switch_VT_1 || sym > XKB_KEY_XF86Switch_VT_12) {
		return false;
	}

	// Nested: no session, no VT
	if(!COMP.session) {
		return true;
	}

	unsigned vt = (unsigned)(sym - XKB_KEY_XF86Switch_VT_1) + 1;
	int current = GetVTNumber();

	// Same VT: libseat switch can freeze, and it's a no-op anyway
	if(current == (int)vt) {
		return true;
	}

	wlr_log(WLR_INFO, "Switching VT %d -> %u", current, vt);
	wlr_session_change_vt(COMP.session, vt);

	// Always consume, the key must not reach clients
	return true;
}
