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
#include "../cleanup/cleanup.h"

#include <lauxlib.h>
#include <lualib.h>
#include <lua.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int parse_mod(const char* mod_str) {
    if(strcmp(mod_str, "Mod1") == 0 || strcmp(mod_str, "Alt") == 0) { return Mod1Mask; }
    if(strcmp(mod_str, "Mod2") == 0) { return Mod2Mask; }
    if(strcmp(mod_str, "Mod3") == 0) { return Mod3Mask; }
    if(strcmp(mod_str, "Mod4") == 0 || strcmp(mod_str, "Super") == 0) { return Mod4Mask; }
    if(strcmp(mod_str, "Mod5") == 0) { return Mod5Mask; }
    if(strcmp(mod_str, "Shift") == 0) { return ShiftMask; }
    if(strcmp(mod_str, "Control") == 0 || strcmp(mod_str, "Ctrl") == 0) { return ControlMask; }
    if(strcmp(mod_str, "Lock") == 0) { return LockMask; }
 
    TraceLog("Unknown modifier: %s", mod_str);
    return 0;
}

int parse_mods_arg(lua_State* lua, int idx) {
    int mods = 0;
 
    if(lua_type(lua, idx) == LUA_TSTRING) {
        mods = parse_mod(luaL_checkstring(lua, idx));
    }
    else if(lua_type(lua, idx) == LUA_TTABLE) {
        size_t n = lua_rawlen(lua, idx);
        for(size_t i = 1; i <= n; i++) {
            lua_rawgeti(lua, idx, (lua_Integer)i);
            mods |= parse_mod(luaL_checkstring(lua, -1));
            lua_pop(lua, 1);
        }
    }
    else {
        luaL_error(lua, "bind: arg %d must be a mod string or table of mod strings", idx);
    }
 
    return mods;
}
 
void parse_keys_arg(lua_State* lua, int idx, KeyBind* bind) {
    const char* name = luaL_checkstring(lua, idx);
    KeySym sym = XStringToKeysym(name);
    if(sym == NoSymbol) {
        TraceLog("Unknown key: %s", name);
    }
 
    bind->key = sym;
}

int l_set(lua_State* lua) {
    const char* key = luaL_checkstring(lua, 1);

    if(strcmp(key, "terminal.command") == 0) {
        const char* val = luaL_checkstring(lua, 2);

        free(DATA.Config.termCommand);
        DATA.Config.termCommand = strdup(val);
		DATA.Config.termCommandArrCount = 0;

		char* tmp = strdup(DATA.Config.termCommand);
    	char* tok = strtok(tmp, " ");
	    while(tok != NULL) {
        	DATA.Config.termCommandArrCount++;
    	    tok = strtok(NULL, " ");
	    }
    	free(tmp);

		if(DATA.Config.termCommandArr) {
    		for(size_t i = 0; i < DATA.Config.termCommandArrCount; i++) {
        		free(DATA.Config.termCommandArr[i]);
    		}
    		free(DATA.Config.termCommandArr);
		}

		DATA.Config.termCommandArr = NULL;
		DATA.Config.termCommandArrCount = 0;
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
		TraceLogFirstLast(true, false, "Terminal array is: ");
		for(size_t i = 0; i < DATA.Config.termCommandArrCount; i++) {
			if(i + 1 != DATA.Config.termCommandArrCount) { TraceLogFirstLast(false, false, "%s ", DATA.Config.termCommandArr[i]); }
			else { TraceLogFirstLast(false, true, "%s", DATA.Config.termCommandArr[i]); }
		}
    }
    else if(strcmp(key, "scale.value") == 0) {
        int dpi = luaL_checkinteger(lua, 2);

        char cmd[128];
        snprintf(cmd, sizeof(cmd), "echo \"Xft.dpi: %d\" | xrdb -merge", dpi);
        system(cmd);

		TraceLog("Set scale to: %d dpi", dpi);
    }
    else if(strncmp(key, "env.", 4) == 0) {
        const char* env = key + 4;
        const char* val = luaL_checkstring(lua, 2);
        setenv(env, val, 1);
		TraceLog("Set env: %s to %s", env, val);
    }
    else if(strcmp(key, "workspaces.minimum") == 0) {
		DATA.Management.minWorkspaces = luaL_checkinteger(lua, 2);
	}

    return 0;
}

int l_bind(lua_State* lua) {
    KeyBind bind = {0};
 
    bind.mods = parse_mods_arg(lua, 1);
 
    parse_keys_arg(lua, 2, &bind);
 
    TraceLog("Bind mods=0x%x key: ", bind.mods, bind.key);
 
    luaL_checktype(lua, 3, LUA_TTABLE);
 
    bind.actionsCount = lua_rawlen(lua, 3);
    bind.actions = malloc(sizeof(*bind.actions) * bind.actionsCount);
    if(!bind.actions) {
        TraceLog("malloc failed :(");
        CleanUp();
        exit(EXIT_FAILURE);
    }
 
    for(size_t i = 0; i < bind.actionsCount; i++) {
        lua_rawgeti(lua, 3, (lua_Integer)(i + 1));
        luaL_checktype(lua, -1, LUA_TTABLE);
 
        bind.actions[i].argc = lua_rawlen(lua, -1);
        TraceLog("Bind %zu argc: %zu", i, bind.actions[i].argc);
 
        bind.actions[i].argv = malloc(sizeof(char*) * (bind.actions[i].argc + 1));
 
        for(size_t j = 0; j < bind.actions[i].argc; j++) {
            lua_rawgeti(lua, -1, (lua_Integer)(j + 1));
            bind.actions[i].argv[j] = strdup(luaL_checkstring(lua, -1));
            lua_pop(lua, 1);
        }
 
        TraceLogFirstLast(true, false, "Bind %zu argv: ", bind.actions[i].argc);
        for(size_t z = 0; z < bind.actions[i].argc; z++) {
            if(z + 1 != bind.actions[i].argc) { TraceLogFirstLast(false, false, "%s ", bind.actions[i].argv[z]); }
            else                               { TraceLogFirstLast(false, true,  "%s", bind.actions[i].argv[z]); }
        }
 
        bind.actions[i].argv[bind.actions[i].argc] = NULL;
 
        lua_pop(lua, 1);
    }
 
    KeyBind* tmp = realloc(DATA.Config.keybinds, sizeof(KeyBind) * (DATA.Config.keybindsCount + 1));
    if(!tmp) {
        TraceLog("realloc failed :(");
        free(bind.actions);
        CleanUp();
        exit(EXIT_FAILURE);
    }
    DATA.Config.keybinds = tmp;
    DATA.Config.keybinds[DATA.Config.keybindsCount] = bind;
    DATA.Config.keybindsCount++;
 
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
	DATA.Config.keybindsCount = 0;
	lua_register(lua, "bind", l_bind);

    if(access(DATA.Config.path, F_OK) != 0) {
        TraceLog("Config not found, generating");
        GenerateConfig();
    }

    if(luaL_dofile(lua, DATA.Config.path) != LUA_OK) {
        TraceLog("Lua config error: %s", lua_tostring(lua, -1));
    }

	lua_close(lua);
}
