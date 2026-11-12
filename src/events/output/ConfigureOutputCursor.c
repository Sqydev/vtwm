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

#include "./output.h"

#include "../../coredata.h"
#include "../input/input.h"

#include <wlr/util/log.h>

void ConfigureOutputCursor(VtwmOutput* output) {
	if(!output || !output->wlr_output) {
		return;
	}

	if(INPUT.cursor_manager && !wlr_xcursor_manager_load(INPUT.cursor_manager, output->wlr_output->scale)) {
		wlr_log(WLR_ERROR, "Failed to load cursor theme for output scale %.2f", output->wlr_output->scale);
	}

	bool lockSoftwareCursor = !CONFIG.cursor.hardware;

	if(output->softwareCursorLocked != lockSoftwareCursor) {
		wlr_output_lock_software_cursors(output->wlr_output, lockSoftwareCursor);
		output->softwareCursorLocked = lockSoftwareCursor;
	}

	if(!INPUT.cursorPosInited) {
		wlr_cursor_warp(INPUT.cursor, NULL, (float)output->wlr_output->width / 2, (float)output->wlr_output->height / 2);
		if(CONFIG.cursor.outputName && strcmp(CONFIG.cursor.outputName, "default") != 0) {
			if(strcmp(output->wlr_output->name, CONFIG.cursor.outputName)) {
				wlr_cursor_warp(INPUT.cursor, NULL, (float)output->wlr_output->width / 2, (float)output->wlr_output->height / 2);
				INPUT.cursorPosInited = true;
			}
		}
	}
}
