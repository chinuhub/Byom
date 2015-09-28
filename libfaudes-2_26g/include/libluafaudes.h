/** @file libluafaudes.h Includes all libluafaudes headers, incl lua  */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008 Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef FAUDES_LIBLUAFAUDES_H
#define FAUDES_LIBLUAFAUDES_H


// lua essentials
#define lua_c

// lua essentials
extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}


// std streams for console logging
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// c libs since we dont use lua.h
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// luafaudes: include faudes plus lua addons
#include "corefaudes.h"
#include "lbp_function.h"

// include declarations for SWIG generated module loaders 
#include "lbp_load.h"

// overall loader to initialize kernel
extern void luafaudes_initialize(lua_State* pL);

// luafaudes: logging externals, defined in commonfaudes.i
extern std::ofstream* luafaudes_logstream;
extern std::string luafaudes_lastline;
extern void luafaudes_logwrite(const char* str);
extern void luafaudes_logwrite(const std::string& str);

// luafaudes: mute faudes console out, see commonfaudes.i
extern void luafaudes_mute(bool on);

// luafaudes: register faudes console variant for print, see commonfaudes.i
extern void luafaudes_print_register(lua_State* L);

// luafaudes interface to completer, see commonfaudes.i
extern char **luafaudes_complete(lua_State* pL, const char *text, int start, int end);

// luafaudes: register faudes LoopCallback() as line hook, see commonfaudes.i
extern void luafaudes_hook_register(lua_State* L);

// luafaudes: dot test external, see commonfaudes.i
extern bool luafaudes_dotready(void); 

// luafaudes: help system, see commonfaudes.i
extern void luafaudes_dict_insert_topic(const std::string& topic, const std::string& text);
extern void luafaudes_dict_insert_entry(const std::string& topic, const std::string& key, const std::string& entry);

// luafaudes: load extension, see commonfaudes.i
extern int luafaudes_loadext(lua_State* pL, const char* filename);
extern int luafaudes_loaddefext(lua_State* pL, const char* arg0);


// luafaudes debugging facilities
#ifdef FAUDES_DEBUG_LUABINDINGS
#define FD_DLB(message) FAUDES_WRITE_CONSOLE("FAUDES_LUA_ERROR: " << message);
#else
#define FD_DLB(message)
#endif



#endif
