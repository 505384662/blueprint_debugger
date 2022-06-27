#include "blueprint_debugger/api/lua_state.h"
#ifdef BLUEPRINT_USE_LUA_SOURCE
#include "lstate.h"
#else
#include "lua-5.4.0/src/lstate.h"
#endif

#include <cstdio>

lua_State *GetMainState_lua54(lua_State *L)
{
	return G(L)->mainthread;
}

std::vector<lua_State *> FindAllCoroutine_lua54(lua_State *L)
{
	std::vector<lua_State *> result;
	auto head = G(L)->allgc;

	while (head)
	{
		if (head->tt == LUA_TTHREAD)
		{
			printf("[BLUEPRINT][FindAllCoroutine_lua54] addr:%p head:%p\n", L, head);
			result.push_back(reinterpret_cast<lua_State *>(head));
		}
		head = head->next;
	}

	return result;
}