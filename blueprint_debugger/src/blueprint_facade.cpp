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

#include "blueprint_debugger/blueprint_facade.h"
#include <cstdarg>
#include <cstdint>
#include "blueprint_debugger/proto/socket_server_transporter.h"
#include "blueprint_debugger/proto/socket_client_transporter.h"
#include "blueprint_debugger/proto/pipeline_server_transporter.h"
#include "blueprint_debugger/proto/pipeline_client_transporter.h"
#include "blueprint_debugger/blueprint_debugger.h"
#include "blueprint_debugger/transporter.h"
#include "blueprint_debugger/blueprint_helper.h"
#include "blueprint_debugger/lua_version.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

BluePrintFacade &BluePrintFacade::Get()
{
	static BluePrintFacade instance;
	return instance;
}

void BluePrintFacade::HookLua(lua_State *L, lua_Debug *ar)
{
	// printf("[BLUEPRINT][BluePrintFacade::HookLua] %p\n", L);
	BluePrintFacade::Get().Hook(L, ar);
}

void BluePrintFacade::ReadyLuaHook(lua_State *L, lua_Debug *ar)
{
	printf("[BLUEPRINT][BluePrintFacade::ReadyLuaHook] addr:%p luaVersion:%d\n", L, luaVersion);
	if (!Get().readyHook && !Get().m_AddLuaState)
	{
		return;
	}

	Get().readyHook = false;

	auto states = FindAllCoroutine(L);

	for (auto state : states)
	{
		printf("[BLUEPRINT][BluePrintFacade::ReadyLuaHook] addr:%p state%p\n", state);
		lua_sethook(state, HookLua, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0);
	}

	lua_sethook(L, HookLua, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0);

	auto debugger = BluePrintFacade::Get().GetDebugger(L);
	if (debugger)
	{
		debugger->Attach();
	}

	Get().Hook(L, ar);
}

BluePrintFacade::BluePrintFacade()
	: transporter(nullptr),
	  isIDEReady(false),
	  isAPIReady(false),
	  isWaitingForIDE(false),
	  StartHook(nullptr),
	  workMode(WorkMode::BluePrintCore),
	  blueprintDebuggerManager(std::make_shared<BluePrintDebuggerManager>()),
	  readyHook(false),
	  m_AddLuaState(false)
{
}

BluePrintFacade::~BluePrintFacade()
{
}

#ifndef BLUEPRINT_USE_LUA_SOURCE
extern "C" bool SetupLuaAPI();

bool BluePrintFacade::SetupLuaAPI()
{
	isAPIReady = ::SetupLuaAPI();
	return isAPIReady;
}

#endif

int LuaError(lua_State *L)
{
	std::string msg = lua_tostring(L, 1);
	msg = "[BLUEPRINT]" + msg;
	lua_getglobal(L, "error");
	lua_pushstring(L, msg.c_str());
	lua_call(L, 1, 0);
	return 0;
}

int LuaPrint(lua_State *L)
{
	std::string msg = lua_tostring(L, 1);
	msg = "[BLUEPRINT]" + msg;
	lua_getglobal(L, "print");
	lua_pushstring(L, msg.c_str());
	lua_call(L, 1, 0);
	return 0;
}

bool BluePrintFacade::TcpListen(lua_State *L, const std::string &host, int port, std::string &err)
{
	Destroy();

	// blueprintDebuggerManager->AddDebugger(L);

	// SetReadyHook(L);

	const auto s = std::make_shared<SocketServerTransporter>();
	transporter = s;
	// s->SetHandler(shared_from_this());
	const auto suc = s->Listen(host, port, err);
	printf("[BLUEPRINT][BluePrintFacade::TcpListen()] host:%s, port:%d\n", host.c_str(), port);
	if (!suc)
	{
		lua_pushcfunction(L, LuaError);
		lua_pushstring(L, err.c_str());
		lua_call(L, 1, 0);
	}
	return suc;
}

bool BluePrintFacade::AddLuaState(lua_State *L)
{
	blueprintDebuggerManager->AddDebugger(L);
	m_AddLuaState = true;
	SetReadyHook(L);
	return true;
}

bool BluePrintFacade::StartDebugServer(lua_State *L, const std::string &host, int port, std::string &err)
{
	if (transporter == nullptr)
	{
		return TcpListen(L, host, port, err);
	}
	return true;
}

bool BluePrintFacade::TcpConnect(lua_State *L, const std::string &host, int port, std::string &err)
{
	Destroy();

	if (!m_AddLuaState)
	{
		blueprintDebuggerManager->AddDebugger(L);
		SetReadyHook(L);
	}

	// blueprintDebuggerManager->AddDebugger(L);
	// SetReadyHook(L);

	const auto c = std::make_shared<SocketClientTransporter>();
	transporter = c;
	// c->SetHandler(shared_from_this());
	const auto suc = c->Connect(host, port, err);
	if (suc)
	{
		WaitIDE(true);
	}
	else
	{
		lua_pushcfunction(L, LuaError);
		lua_pushstring(L, err.c_str());
		lua_call(L, 1, 0);
	}
	return suc;
}

bool BluePrintFacade::PipeListen(lua_State *L, const std::string &name, std::string &err)
{
	Destroy();

	blueprintDebuggerManager->AddDebugger(L);

	SetReadyHook(L);

	const auto p = std::make_shared<PipelineServerTransporter>();
	transporter = p;
	// p->SetHandler(shared_from_this());
	const auto suc = p->pipe(name, err);
	return suc;
}

bool BluePrintFacade::PipeConnect(lua_State *L, const std::string &name, std::string &err)
{
	Destroy();

	blueprintDebuggerManager->AddDebugger(L);

	SetReadyHook(L);

	const auto p = std::make_shared<PipelineClientTransporter>();
	transporter = p;
	// p->SetHandler(shared_from_this());
	const auto suc = p->Connect(name, err);
	if (suc)
	{
		WaitIDE(true);
	}
	return suc;
}

void BluePrintFacade::WaitIDE(bool force, int timeout)
{
	if (transporter != nullptr && (transporter->IsServerMode() || force) && !isWaitingForIDE && !isIDEReady)
	{
		isWaitingForIDE = true;
		std::unique_lock<std::mutex> lock(waitIDEMutex);
		if (timeout > 0)
			waitIDECV.wait_for(lock, std::chrono::milliseconds(timeout));
		else
			waitIDECV.wait(lock);
		isWaitingForIDE = false;
	}
}

int BluePrintFacade::BreakHere(lua_State *L)
{
	if (!isIDEReady)
		return 0;

	blueprintDebuggerManager->HandleBreak(L);

	return 1;
}

int BluePrintFacade::OnConnect(bool suc)
{
	return 0;
}

int BluePrintFacade::OnDisconnect()
{
	isIDEReady = false;
	isWaitingForIDE = false;

	blueprintDebuggerManager->OnDisconnect();

	if (workMode == WorkMode::Attach)
	{
		blueprintDebuggerManager->RemoveAllDebugger();
	}

	return 0;
}

void BluePrintFacade::Destroy()
{
	OnDisconnect();

	if (transporter)
	{
		transporter->Stop();
		transporter = nullptr;
	}
}

void BluePrintFacade::SetWorkMode(WorkMode mode)
{
	workMode = mode;
}

WorkMode BluePrintFacade::GetWorkMode()
{
	return workMode;
}

void BluePrintFacade::OnReceiveMessage(const rapidjson::Document &document)
{
	const auto cmd = static_cast<MessageCMD>(document["cmd"].GetInt());
	printf("[BLUEPRINT][BluePrintFacade::OnReceiveMessage()] cmd:%d\n", cmd);
	switch (cmd)
	{
	case MessageCMD::InitReq:
		OnInitReq(document);
		break;
	case MessageCMD::ReadyReq:
		OnReadyReq(document);
		break;
	case MessageCMD::AddBreakPointReq:
		OnAddBreakPointReq(document);
		break;
	case MessageCMD::RemoveBreakPointReq:
		OnRemoveBreakPointReq(document);
		break;
	case MessageCMD::ActionReq:
		// assert(isIDEReady);
		OnActionReq(document);
		break;
	case MessageCMD::EvalReq:
		// assert(isIDEReady);
		OnEvalReq(document);
		break;
	case MessageCMD::SetVariableReq:
		OnVariableReq(document);
		break;
	default:
		break;
	}
}

void BluePrintFacade::OnInitReq(const rapidjson::Document &document)
{
	if (StartHook)
	{
		StartHook();
	}

	if (document.HasMember("blueprintHelper"))
	{
		blueprintDebuggerManager->helperCode = document["blueprintHelper"].GetString();
	}

	// file extension names: .lua, .txt, .lua.txt ...
	if (document.HasMember("ext"))
	{
		std::vector<std::string> extNames;
		const auto ext = document["ext"].GetArray();
		auto it = ext.begin();
		while (it != ext.end())
		{
			const auto extName = (*it).GetString();
			extNames.emplace_back(extName);
			++it;
		}

		blueprintDebuggerManager->extNames = extNames;
	}

	// 这里有个线程安全问题，消息线程和lua 执行线程不是相同线程，但是没有一个锁能让我做同步
	// 所以我不能在这里访问lua state 指针的内部结构
	//
	// 方案：提前为主state 设置hook 利用hook 实现同步

	// fix 以上安全问题
	StartDebug();
}

void BluePrintFacade::OnReadyReq(const rapidjson::Document &document)
{
	isIDEReady = true;
	waitIDECV.notify_all();
}

void FillVariables(rapidjson::Value &container, const std::vector<std::shared_ptr<Variable>> &variables,
				   rapidjson::MemoryPoolAllocator<> &allocator);

void FillVariable(rapidjson::Value &container, const std::shared_ptr<Variable> variable,
				  rapidjson::MemoryPoolAllocator<> &allocator)
{
	container.AddMember("name", variable->name, allocator);
	container.AddMember("nameType", variable->nameType, allocator);
	container.AddMember("value", variable->value, allocator);
	container.AddMember("valueType", variable->valueType, allocator);
	container.AddMember("valueTypeName", variable->valueTypeName, allocator);
	container.AddMember("cacheId", variable->cacheId, allocator);
	container.AddMember("nVar", variable->nVar, allocator);
	container.AddMember("frameId", variable->frameId, allocator);
	// children
	if (!variable->children.empty())
	{
		rapidjson::Value childrenValue(rapidjson::kArrayType);
		FillVariables(childrenValue, variable->children, allocator);
		container.AddMember("children", childrenValue, allocator);
	}
}

void FillVariables(rapidjson::Value &container, const std::vector<std::shared_ptr<Variable>> &variables,
				   rapidjson::MemoryPoolAllocator<> &allocator)
{
	std::vector<std::shared_ptr<Variable>> tmp = variables;
	for (auto variable : tmp)
	{
		rapidjson::Value variableValue(rapidjson::kObjectType);
		FillVariable(variableValue, variable, allocator);
		container.PushBack(variableValue, allocator);
	}
}

void FillStacks(rapidjson::Value &container, std::vector<std::shared_ptr<Stack>> &stacks,
				rapidjson::MemoryPoolAllocator<> &allocator)
{
	for (auto stack : stacks)
	{
		rapidjson::Value stackValue(rapidjson::kObjectType);
		stackValue.AddMember("file", stack->file, allocator);
		stackValue.AddMember("functionName", stack->functionName, allocator);
		stackValue.AddMember("line", stack->line, allocator);
		stackValue.AddMember("frameId", stack->frameId, allocator);

		// local variables
		rapidjson::Value localVariables(rapidjson::kArrayType);
		FillVariables(localVariables, stack->localVariables, allocator);
		stackValue.AddMember("localVariables", localVariables, allocator);
		// upvalue variables
		rapidjson::Value upvalueVariables(rapidjson::kArrayType);
		FillVariables(upvalueVariables, stack->upvalueVariables, allocator);
		stackValue.AddMember("upvalueVariables", upvalueVariables, allocator);

		container.PushBack(stackValue, allocator);
	}
}

bool BluePrintFacade::OnBreak(std::shared_ptr<Debugger> debugger)
{
	if (!debugger)
	{
		return false;
	}
	std::vector<std::shared_ptr<Stack>> stacks;

	blueprintDebuggerManager->SetBreakedDebugger(debugger);

	debugger->GetStacks(stacks, []()
						{ return std::make_shared<Stack>(); });

	rapidjson::Document document;
	document.SetObject();
	auto &allocator = document.GetAllocator();
	document.AddMember("cmd", static_cast<int>(MessageCMD::BreakNotify), allocator);
	// stacks
	rapidjson::Value stacksValue(rapidjson::kArrayType);
	FillStacks(stacksValue, stacks, allocator);
	document.AddMember("stacks", stacksValue, allocator);

	for (auto stack : stacks)
	{
		printf("[BLUEPRINT][BluePrintFacade::OnBreak()]  file:%s, line:%d ,frameId:%d, localValue:%d, upValue:%d\n", stack->file.c_str(), stack->line, stack->frameId, stack->localVariables.size(), stack->upvalueVariables.size());
	}

	transporter->Send(int(MessageCMD::BreakNotify), document);

	return true;
}

void ReadBreakPoint(const rapidjson::Value &value, std::shared_ptr<BreakPoint> bp)
{
	if (value.HasMember("file"))
	{
		bp->file = value["file"].GetString();
	}
	if (value.HasMember("line"))
	{
		bp->line = value["line"].GetInt();
	}
	if (value.HasMember("condition"))
	{
		bp->condition = value["condition"].GetString();
	}
	if (value.HasMember("hitCondition"))
	{
		bp->hitCondition = value["hitCondition"].GetString();
	}
	if (value.HasMember("logMessage"))
	{
		bp->logMessage = value["logMessage"].GetString();
	}
}

void BluePrintFacade::OnAddBreakPointReq(const rapidjson::Document &document)
{
	if (document.HasMember("clear"))
	{
		const auto all = document["clear"].GetBool();
		if (all)
		{
			blueprintDebuggerManager->RemoveAllBreakPoints();
		}
	}
	if (document.HasMember("breakPoints"))
	{
		const auto docBreakPoints = document["breakPoints"].GetArray();
		auto it = docBreakPoints.begin();
		while (it != docBreakPoints.end())
		{
			auto bp = std::make_shared<BreakPoint>();
			ReadBreakPoint(*it, bp);

			bp->hitCount = 0;
			// ParsePathParts(bp->file, bp->pathParts);

			blueprintDebuggerManager->AddBreakpoint(bp);

			++it;
		}
	}
	// todo: response
}

void BluePrintFacade::OnRemoveBreakPointReq(const rapidjson::Document &document)
{
	if (document.HasMember("breakPoints"))
	{
		const auto breakPoints = document["breakPoints"].GetArray();
		auto it = breakPoints.begin();
		while (it != breakPoints.end())
		{
			auto bp = std::make_shared<BreakPoint>();
			ReadBreakPoint(*it, bp);
			blueprintDebuggerManager->RemoveBreakpoint(bp->file, bp->line);
			++it;
		}
	}
	// todo: response
}

void BluePrintFacade::OnActionReq(const rapidjson::Document &document)
{
	const auto action = static_cast<DebugAction>(document["action"].GetInt());

	blueprintDebuggerManager->DoAction(action);
	// todo: response
}

bool BluePrintFacade::isnum(string s)
{
	stringstream sin(s);
	double t;
	char p;
	if (!(sin >> t))
		return false;
	if (sin >> p)
		return false;
	else
		return true;
}

void BluePrintFacade::SendVariableRsp(bool success, string name, string type, string value, string variablesReference, string tip, bool findVar)
{
	rapidjson::Document rspDoc;
	rspDoc.SetObject();
	auto &allocator = rspDoc.GetAllocator();
	rspDoc.AddMember("success", success, allocator);

	if (findVar)
	{
		if (success)
		{
			rspDoc.AddMember("name", name, allocator);
			rspDoc.AddMember("type", type, allocator);
			rspDoc.AddMember("value", value, allocator);
			rspDoc.AddMember("variablesReference", variablesReference, allocator);
			rspDoc.AddMember("tip", "set success", allocator);
		}
		else
		{
			rspDoc.AddMember("tip", "find data but set fail", allocator);
		}
	}
	else
	{
		rspDoc.AddMember("tip", "find not data", allocator);
	}

	if (transporter)
	{
		transporter->Send(int(MessageCMD::EvalRsp), rspDoc);
	}
}

void BluePrintFacade::OnVariableReq(const rapidjson::Document &document)
{
	const auto currentFrameId = document["currentFrameId"].GetInt();
	const std::string commandData = document["commandData"].GetString();

	blueprintDebuggerManager->SetValueByCommand(currentFrameId, commandData);
}

void BluePrintFacade::OnEvalReq(const rapidjson::Document &document)
{
	const auto seq = document["seq"].GetInt();
	const auto expr = document["expr"].GetString();
	const auto stackLevel = document["stackLevel"].GetInt();
	const auto depth = document["depth"].GetInt();
	auto cacheId = 0;
	if (document.HasMember("cacheId"))
	{
		cacheId = document["cacheId"].GetInt();
	}

	auto context = std::make_shared<EvalContext>();
	context->seq = seq;
	context->expr = expr;
	context->stackLevel = stackLevel;
	context->depth = depth;
	context->cacheId = cacheId;
	context->success = false;

	printf("[BLUEPRINT][BluePrintFacade::OnEvalReq] context->expr:%s\n", context->expr.c_str());

	blueprintDebuggerManager->Eval(context);
}

void BluePrintFacade::OnEvalResult(std::shared_ptr<EvalContext> context)
{
	rapidjson::Document rspDoc;
	rspDoc.SetObject();
	auto &allocator = rspDoc.GetAllocator();
	rspDoc.AddMember("seq", context->seq, allocator);
	rspDoc.AddMember("success", context->success, allocator);
	if (context->success)
	{
		rapidjson::Value v(rapidjson::kObjectType);
		FillVariable(v, context->result, allocator);
		rspDoc.AddMember("value", v, allocator);
	}
	else
	{
		rspDoc.AddMember("error", context->error, allocator);
	}

	if (transporter)
	{
		transporter->Send(int(MessageCMD::EvalRsp), rspDoc);
	}
}

void BluePrintFacade::SendLog(LogType type, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buff[1024] = {0};
	vsnprintf(buff, 1024, fmt, args);
	va_end(args);

	const std::string msg = buff;

	rapidjson::Document rspDoc;
	rspDoc.SetObject();
	auto &allocator = rspDoc.GetAllocator();
	rspDoc.AddMember("type", (int)type, allocator);
	rspDoc.AddMember("message", msg, allocator);
	if (transporter)
	{
		transporter->Send(int(MessageCMD::LogNotify), rspDoc);
	}
}

void BluePrintFacade::OnLuaStateGC(lua_State *L)
{
	auto debugger = blueprintDebuggerManager->RemoveDebugger(L);

	if (debugger)
	{
		debugger->Detach();
	}

	if (workMode == WorkMode::BluePrintCore)
	{
		if (blueprintDebuggerManager->IsDebuggerEmpty())
		{
			Destroy();
		}
	}
}

void BluePrintFacade::Hook(lua_State *L, lua_Debug *ar)
{
	// printf("[BLUEPRINT][BluePrintFacade::Hook] %p\n", L);
	auto debugger = GetDebugger(L);
	if (debugger)
	{
		if (!debugger->IsRunning())
		{
			if (BluePrintFacade::Get().GetWorkMode() == WorkMode::BluePrintCore)
			{
				if (luaVersion != LuaVersion::LUA_JIT)
				{
					if (debugger->IsMainCoroutine(L))
					{
						SetReadyHook(L);
					}
				}
				else
				{
					SetReadyHook(L);
				}
			}
			return;
		}

		debugger->Hook(ar, L);
	}
	else
	{
		if (workMode == WorkMode::Attach)
		{
			debugger = blueprintDebuggerManager->AddDebugger(L);
			install_blueprint_core(L);
			if (blueprintDebuggerManager->IsRunning())
			{
				debugger->Start();
				debugger->Attach();
			}
			// send attached notify
			rapidjson::Document rspDoc;
			rspDoc.SetObject();
			// fix macosx compiler error,
			// repidjson 应该有重载决议的错误
			int64_t state = reinterpret_cast<int64_t>(L);
			rspDoc.AddMember("state", state, rspDoc.GetAllocator());
			this->transporter->Send(int(MessageCMD::AttachedNotify), rspDoc);

			debugger->Hook(ar, L);
		}
	}
}

std::shared_ptr<BluePrintDebuggerManager> BluePrintFacade::GetDebugManager() const
{
	return blueprintDebuggerManager;
}

std::shared_ptr<Variable> BluePrintFacade::GetVariableRef(Variable *variable)
{
	auto it = luaVariableRef.find(variable);

	if (it != luaVariableRef.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}

void BluePrintFacade::AddVariableRef(std::shared_ptr<Variable> variable)
{
	luaVariableRef.insert({variable.get(), variable});
}

void BluePrintFacade::RemoveVariableRef(Variable *variable)
{
	auto it = luaVariableRef.find(variable);
	if (it != luaVariableRef.end())
	{
		luaVariableRef.erase(it);
	}
}

std::shared_ptr<Debugger> BluePrintFacade::GetDebugger(lua_State *L)
{
	return blueprintDebuggerManager->GetDebugger(L);
}

void BluePrintFacade::SetReadyHook(lua_State *L)
{
	lua_sethook(L, ReadyLuaHook, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0);
}

void BluePrintFacade::StartDebug()
{
	blueprintDebuggerManager->SetRunning(true);
	printf("[BLUEPRINT][BluePrintFacade::StartDebug] true\n");
	readyHook = true;
}

void BluePrintFacade::StartupHookMode(int port)
{
	Destroy();

	// 1024 - 65535
	while (port > 0xffff)
		port -= 0xffff;
	while (port < 0x400)
		port += 0x400;

	const auto s = std::make_shared<SocketServerTransporter>();
	std::string err;
	const auto suc = s->Listen("localhost", port, err);
	if (suc)
	{
		transporter = s;
		// transporter->SetHandler(shared_from_this());
	}
}

void BluePrintFacade::Attach(lua_State *L)
{
	if (!this->transporter->IsConnected())
		return;

	// 这里存在一个问题就是 hook 的时机太早了，globalstate 都还没初始化完毕

	if (!isAPIReady)
	{
		// 考虑到blueprint_hook use lua source
		isAPIReady = install_blueprint_core(L);
	}

	lua_sethook(L, BluePrintFacade::HookLua, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0);
}
