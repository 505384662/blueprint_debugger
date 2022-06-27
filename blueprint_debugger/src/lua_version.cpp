#include "blueprint_debugger/lua_version.h"

#if BLUEPRINT_LUA_51
LuaVersion luaVersion = LuaVersion::LUA_51;
#elif BLUEPRINT_LUA_52
LuaVersion luaVersion = LuaVersion::LUA_52;
#elif BLUEPRINT_LUA_53
LuaVersion luaVersion = LuaVersion::LUA_53;
#elif BLUEPRINT_LUA_54
LuaVersion luaVersion = LuaVersion::LUA_54;
#elif BLUEPRINT_LUA_JIT
LuaVersion luaVersion = LuaVersion::LUA_JIT;
#else
LuaVersion luaVersion = LuaVersion::UNKNOWN;
#endif
