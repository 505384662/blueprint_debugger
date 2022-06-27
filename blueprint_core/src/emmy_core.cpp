/*
 * Copyright (c) 2019. tangzx(love.tangzx@qq.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "blueprint_core/blueprint_core.h"
#include "blueprint_debugger/blueprint_helper.h"
#include "blueprint_debugger/blueprint_facade.h"

static const luaL_Reg lib[] = {
	{"tcpListen", tcpListen},
	{"tcpConnect", tcpConnect},
	{"pipeListen", pipeListen},
	{"pipeConnect", pipeConnect},
	{"waitIDE", waitIDE},
	{"breakHere", breakHere},
	{"stop", stop},
	{"tcpSharedListen", tcpSharedListen},
	{"addLuaState", addLuaState},
	{nullptr, nullptr}};

extern "C"
{
	BLUEPRINT_CORE_EXPORT int luaopen_blueprint_core(struct lua_State *L)
	{
		BluePrintFacade::Get().SetWorkMode(WorkMode::BluePrintCore);
		if (!install_blueprint_core(L))
			return false;
		luaL_newlibtable(L, lib);
		luaL_setfuncs(L, lib, 0);

		// _G.blueprint_core
		lua_pushglobaltable(L);
		lua_pushstring(L, "blueprint_core");
		lua_pushvalue(L, -3);
		lua_rawset(L, -3);
		lua_pop(L, 1);

		return 1;
	}
}
