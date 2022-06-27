#include "blueprint_debugger/api/lua_state.h"
#include "blueprint_debugger/lua_version.h"
#include <cstdio>

#ifdef BLUEPRINT_USE_LUA_SOURCE
#if defined(BLUEPRINT_LUA_JIT)
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__) return __LuaJIT__
#elif defined(BLUEPRINT_LUA_51)
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__, __Default__) return __Lua51__
#elif defined(BLUEPRINT_LUA_52)
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__, __Default__) return __Lua52__
#elif defined(BLUEPRINT_LUA_53)
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__, __Default__) return __Lua53__
#elif defined(BLUEPRINT_LUA_54)
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__, __Default__) return __Lua54__
#endif
#else
#define LuaSwitchDo(__LuaJIT__, __Lua51__, __Lua52__, __Lua53__, __Lua54__, __Default__) \
	switch (luaVersion)                                                                  \
	{                                                                                    \
	case LuaVersion::LUA_JIT:                                                            \
	{                                                                                    \
		return __LuaJIT__;                                                               \
	}                                                                                    \
	case LuaVersion::LUA_51:                                                             \
	{                                                                                    \
		return __Lua51__;                                                                \
	}                                                                                    \
	case LuaVersion::LUA_52:                                                             \
	{                                                                                    \
		return __Lua52__;                                                                \
	}                                                                                    \
	case LuaVersion::LUA_53:                                                             \
	{                                                                                    \
		return __Lua53__;                                                                \
	}                                                                                    \
	case LuaVersion::LUA_54:                                                             \
	{                                                                                    \
		return __Lua54__;                                                                \
	}                                                                                    \
	default:                                                                             \
		return __Default__;                                                              \
	}
#endif

lua_State *GetMainState(lua_State *L)
{
	LuaSwitchDo(
		GetMainState_luaJIT(L),
		GetMainState_lua51(L),
		GetMainState_lua52(L),
		GetMainState_lua53(L),
		GetMainState_lua54(L),
		nullptr);
}

std::vector<lua_State *> FindAllCoroutine(lua_State *L)
{
	printf("[BLUEPRINT][LuaSwitchDo] FindAllCoroutine:%d\n", luaVersion);
	LuaSwitchDo(
		std::vector<lua_State *>(),
		FindAllCoroutine_lua51(L),
		FindAllCoroutine_lua52(L),
		FindAllCoroutine_lua53(L),
		FindAllCoroutine_lua54(L),
		std::vector<lua_State *>());
}
