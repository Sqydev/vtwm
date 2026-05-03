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
#include "../coredata.h"
#include "../logging/logging.h"

#include <lauxlib.h>
#include <lualib.h>
#include <lua.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int l_set(lua_State* lua) {
    const char* key = luaL_checkstring(lua, 1);

    if(strcmp(key, "terminal.command") == 0) {
        const char* val = luaL_checkstring(lua, 2);

        free(DATA.Config.termCommand);
        DATA.Config.termCommand = strdup(val);

		char* tmp = strdup(DATA.Config.termCommand);
    	char* tok = strtok(tmp, " ");
	    while(tok != NULL) {
        	DATA.Config.termCommandArrCount++;
    	    tok = strtok(NULL, " ");
	    }
    	free(tmp);

	    DATA.Config.termCommandArr = malloc((DATA.Config.termCommandArrCount + 1) * sizeof(char*));

    	char* termCopy = strdup(DATA.Config.termCommand);
	    char* token = strtok(termCopy, " ");

    	size_t i = 0;
	    while(token != NULL) {
        	DATA.Config.termCommandArr[i] = strdup(token);
    	    i++;
	        token = strtok(NULL, " ");
	    }
	    DATA.Config.termCommandArr[i] = NULL;

		free(termCopy);

		TraceLog("Terminal command is: %s", DATA.Config.termCommand);
    }
    else if(strcmp(key, "scale.value") == 0) {
        int dpi = luaL_checkinteger(lua, 2);

        char cmd[128];
        snprintf(cmd, sizeof(cmd), "echo \"Xft.dpi: %d\" | xrdb -merge", dpi);
        system(cmd);
    }
    else if(strncmp(key, "env.", 4) == 0) {
        const char* env = key + 4;
        const char* val = luaL_checkstring(lua, 2);
        setenv(env, val, 1);
    }

    return 0;
}

void LoadConfig(void) {
    lua_State* lua = luaL_newstate();

    luaL_requiref(lua, "_G", luaopen_base, 1); lua_pop(lua, 1);
    luaL_requiref(lua, LUA_TABLIBNAME, luaopen_table, 1); lua_pop(lua, 1);
    luaL_requiref(lua, LUA_STRLIBNAME, luaopen_string, 1); lua_pop(lua, 1);
    luaL_requiref(lua, LUA_MATHLIBNAME, luaopen_math, 1); lua_pop(lua, 1);

    lua_pushnil(lua); lua_setglobal(lua, "dofile");
    lua_pushnil(lua); lua_setglobal(lua, "load");
    lua_pushnil(lua); lua_setglobal(lua, "loadfile");

    lua_register(lua, "set", l_set);

    if(access(DATA.Config.path, F_OK) != 0) {
        TraceLog("Config not found, generating");
        GenerateConfig();
    }

    if(luaL_dofile(lua, DATA.Config.path) != LUA_OK) {
        TraceLog("Lua config error: %s", lua_tostring(lua, -1));
    }
}
