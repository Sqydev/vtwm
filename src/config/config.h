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

#ifndef VTWM_CONFIG_H
#define VTWM_CONFIG_H

#include "../wm/wm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CONFIG_LAYER_COUNT (LAYER_SUPER + 1)

typedef enum ConfigModifier {
	CONFIG_MODIFIER_SUPER = 0,
	CONFIG_MODIFIER_CTRL,
	CONFIG_MODIFIER_ALT,
	CONFIG_MODIFIER_SHIFT,
	CONFIG_MODIFIER_COUNT
} ConfigModifier;

typedef struct ConfigLayers {
	bool enabled[CONFIG_LAYER_COUNT];
} ConfigLayers;

typedef struct ConfigModifiers {
	bool enabled[CONFIG_MODIFIER_COUNT];
} ConfigModifiers;

typedef struct ConfigMargin {
	int top;
	int bottom;
	int left;
	int right;
} ConfigMargin;

typedef struct ConfigBorder {
	uint32_t focusedColor;
	uint32_t unFocusedColor;
	int radius;
} ConfigBorder;

typedef struct ConfigLooks {
	bool defined;

	ConfigMargin margin;
	ConfigBorder border;
} ConfigLooks;

typedef struct ConfigCursor {
	char* theme;
	bool hardware;
	int scale;
} ConfigCursor;

typedef struct ConfigInput {
	char* layout;
	int repeatRate;
	int repeatDelay;
} ConfigInput;

typedef struct ConfigStartUp {
	char** commands;
	size_t commandCount;
} ConfigStartUp;

typedef struct ConfigKeybind {
	char* binding;
	ConfigLayers layers;
	ConfigModifiers modifiers;

	char* key;
	char* action;
} ConfigKeybind;

typedef struct Config {
	char* configPath;

	char* terminalCommand;
	char** terminalCommandSliced;

	ConfigLooks looks[CONFIG_LAYER_COUNT];

	ConfigCursor cursor;
	ConfigInput input;
	ConfigStartUp startup;

	ConfigKeybind* keybinds;
	size_t keybindCount;
} Config;

ConfigLayers ConfigLayersAll(void);
bool ConfigLayersAny(const ConfigLayers* layers);
bool ConfigModifiersEqual(const ConfigModifiers* a, const ConfigModifiers* b);

bool LoadConfig(void);
bool ReloadConfig(void);
void ParseConfig(const char* data, size_t size);
void UnloadConfig(void);

#endif
