#pragma once

#ifndef RMLUI_LUA_AS_CXX
extern "C" {
#endif

// The standard Lua headers
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#ifndef RMLUI_LUA_AS_CXX
}
#endif

// Lua 5.1 does not provide luaL_testudata. Keep the Lua bindings compatible
// with the Lua headers used by consumers of RmlUi.
#if LUA_VERSION_NUM == 501
inline void* luaL_testudata(lua_State* L, int argument, const char* type_name)
{
	void* userdata = lua_touserdata(L, argument);
	if (userdata && lua_getmetatable(L, argument))
	{
		lua_getfield(L, LUA_REGISTRYINDEX, type_name);
		const int matches = lua_rawequal(L, -1, -2);
		lua_pop(L, 2);
		if (matches)
			return userdata;
	}
	return nullptr;
}
#endif
