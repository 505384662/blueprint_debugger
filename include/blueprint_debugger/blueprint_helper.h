#pragma once

#include "api/lua_api.h"
#include "types.h"

bool query_variable(lua_State *L, std::shared_ptr<Variable> variable, const char *typeName, int object, int depth);

// blueprint.tcpListen(host: string, port: int): bool
int tcpListen(struct lua_State *L);

// blueprint.tcpConnect(host: string, port: int): bool
int tcpConnect(lua_State *L);

// blueprint.pipeListen(pipeName: string): bool
int pipeListen(lua_State *L);

// blueprint.pipeConnect(pipeName: string): bool
int pipeConnect(lua_State *L);

// blueprint.breakHere(): bool
int breakHere(lua_State *L);

// blueprint.waitIDE(timeout: number): void
int waitIDE(lua_State *L);

int tcpSharedListen(lua_State *L);

int addLuaState(struct lua_State *L);

// blueprint.stop()
int stop(lua_State *L);

bool install_blueprint_core(struct lua_State *L);

/*
 * @deprecated
 */
void ParsePathParts(const std::string &file, std::vector<std::string> &paths);

// 等于0表示相等
bool CompareIgnoreCase(const std::string &lh, const std::string &rh);

struct CaseInsensitiveLess final
{
	bool operator()(const std::string &lhs, const std::string &rhs) const;
};

// 直到C++ 20才有endwith，这里自己写一个
bool EndWith(const std::string &source, const std::string &end);

std::string BaseName(const std::string &filePath);
