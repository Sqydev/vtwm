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

#include "./config.h"
#include "../headers/coredata.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* trim(char* text) {
	while(isspace((unsigned char) *text)) {
		text++;
	}

	char* end = text + strlen(text);

	while(end > text && isspace((unsigned char) *(end - 1))) {
		end--;
	}

	*end = '\0';
	return text;
}

char* unquote(char* text) {
	if(*text != '"') {
		return text;
	}

	text++;

	char* closingQuote = strrchr(text, '"');
	if(closingQuote) {
    	*closingQuote = '\0';
	}

	return text;
}

typedef struct {
    const char* section;
    const char* key;
    void (*apply)(const char* key, const char* value);
} SetHandler;

void apply_terminal_command(const char* key, const char* value) {
	(void)key;

    free(DATA.Config.termCommand);
    DATA.Config.termCommand = strdup(value);
}

void set_xrbd_scale(const char* key, const char* value) {
	(void)key;

	char* xrdbComm;
	if(asprintf(&xrdbComm, "echo \"Xft.dpi: %s\" | xrdb -merge", value) == -1) {
		return;
	}

	system(xrdbComm);
	free(xrdbComm);
}

void set_env(const char* env, const char* value) {
	setenv(env, value, 1);
}

const SetHandler SET_HANDLERS[] = {
    { "terminal", "command", apply_terminal_command },
    { "scale", "value", set_xrbd_scale },
    { "env", "*", set_env },
};

void handle_set(char* args) {
    char* equalsSign = strchr(args, '=');
    if(!equalsSign) {
        return;
	}

    *equalsSign = '\0';

    char* left_side = trim(args);
    char* value = trim(equalsSign + 1);

    value = unquote(value);

    char* dot = strchr(left_side, '.');
    if(!dot) {
        return;
	}

    *dot = '\0';

    const char* section = left_side;
    const char* key = dot + 1;

    for(size_t i = 0; i < sizeof(SET_HANDLERS) / sizeof(*SET_HANDLERS); i++) {
        if(strcmp(SET_HANDLERS[i].section, section) == 0 && (strcmp(SET_HANDLERS[i].key, key) == 0 || strcmp(SET_HANDLERS[i].key, "*") == 0)) {
            SET_HANDLERS[i].apply(key, value);
            return;
        }
    }
}

bool LoadConfig(void) {
    FILE* config_file = fopen(DATA.Config.path, "r");
    if(!config_file) {
        return false;
	}

    char* line = NULL;
    size_t line_capacity = 0;

    while(getline(&line, &line_capacity, config_file) != -1) {
        char* trimmed = trim(line);

        if(*trimmed == '\0' || *trimmed == '#') {
            continue;
		}

        if(strncmp(trimmed, "set ", 4) == 0) {
            handle_set(trimmed + 4);
            continue;
        }
    }

    free(line);
    fclose(config_file);
    return true;
}
