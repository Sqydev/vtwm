/* 
* Copyright (c) 2025-present W.K. ( _Sqyd_ / Sqydev )
* Github: https://github.com/Sqydev
* GPG Fingerprint: 6DC2516B0DFDA9C59661650722F7B8A777F33B56
*
* This software is provided "as-is", without any express or implied warranty. In no event
* will the authors be held liable for any damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose, including commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
* 1. Non-Misrepresentation: The origin of this software must not be misrepresented; 
*    you must not claim that you wrote the original software. An acknowledgment in 
*    product documentation is appreciated but not required.
*
* 2. Source-Level Copyleft: Any altered versions (forks) of this software's source code, 
*    or files containing significant portions of this code, must be distributed under 
*    these same license terms. Such modified source code must be made publicly available 
*    to any recipient, even if used over a network (SaaS).
* 
* 3. Proprietary Integration: This software may be integrated into, linked with, or 
*    used as a component of proprietary and closed-source products. In such cases, 
*    the surrounding proprietary application code does not need to be disclosed, 
*    provided that the original or modified source code of THIS software remains 
*    available under the terms of Section 2.
*
* 4. Persistent Metadata: All original credits, including those in the source code headers 
*    and binary metadata (e.g., ELF .comment section, PE StringFileInfo, or equivalent), 
*    must not be removed. You may add your own credits to forks, provided the original 
*    authorship remains clearly identified.
* 
* 5. Notice Retention: This license notice may not be removed or altered from any 
*    source or binary distribution.
*/

#include "../xdg.h"

#include "../../../utils/utils.h"

#include <stdlib.h>

void NewToplevelDecoration(struct wl_listener* listener, void* data) {
    (void)listener;
    struct wlr_xdg_toplevel_decoration_v1* decoration = data;

    VtwmToplevelDecoration* deco = IFNULLPANIC(calloc(1, sizeof(*deco)), 1, "Failed to allocate decoration\n");
    deco->decoration = decoration;

    deco->commit.notify = CommitDecoration;
    wl_signal_add(&decoration->toplevel->base->surface->events.commit, &deco->commit);

    deco->request_mode.notify = RequestMode;
    wl_signal_add(&decoration->events.request_mode, &deco->request_mode);

    deco->destroy.notify = DestroyDecoration;
    wl_signal_add(&decoration->events.destroy, &deco->destroy);

    if(decoration->toplevel->base->initialized) {
        wlr_xdg_toplevel_decoration_v1_set_mode(deco->decoration, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
}
