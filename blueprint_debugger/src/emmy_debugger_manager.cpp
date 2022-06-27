#include "blueprint_debugger/blueprint_debugger_manager.h"
#include "blueprint_debugger/blueprint_helper.h"
#include "blueprint_debugger/lua_version.h"

BluePrintDebuggerManager::BluePrintDebuggerManager()
	: stateBreak(std::make_shared<HookStateBreak>()),
	  stateContinue(std::make_shared<HookStateContinue>()),
	  stateStepOver(std::make_shared<HookStateStepOver>()),
	  stateStop(std::make_shared<HookStateStop>()),
	  stateStepIn(std::make_shared<HookStateStepIn>()),
	  stateStepOut(std::make_shared<HookStateStepOut>()),
	  isRunning(false)
{
}

BluePrintDebuggerManager::~BluePrintDebuggerManager()
{
}

std::shared_ptr<Debugger> BluePrintDebuggerManager::GetDebugger(lua_State *L)
{
	std::lock_guard<std::mutex> lock(debuggerMtx);
	auto identify = GetUniqueIdentify(L);
	auto it = debuggers.find(identify);
	if (it != debuggers.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<Debugger> BluePrintDebuggerManager::AddDebugger(lua_State *L)
{
	std::lock_guard<std::mutex> lock(debuggerMtx);

	auto identify = GetUniqueIdentify(L);

	std::shared_ptr<Debugger> debugger = nullptr;

	auto it = debuggers.find(identify);

	if (it == debuggers.end())
	{
		if (luaVersion != LuaVersion::LUA_JIT)
		{
			debugger = std::make_shared<Debugger>(reinterpret_cast<lua_State *>(identify), shared_from_this());
		}
		else
		{
			// 如果首次add 的state不是main state，则main state视为空指针
			// 但不影响luajit附加调试和远程调试
			lua_State *mainState = nullptr;

			int ret = lua_pushthread(L);
			lua_pop(L, 1);
			if (ret == 1)
			{
				mainState = L;
			}

			debugger = std::make_shared<Debugger>(mainState, shared_from_this());
		}

		debuggers.insert({identify, debugger});
		printf("[BLUEPRINT][BluePrintDebuggerManager::AddDebugger()] luaStateIdentify:%lld\n", identify);
	}
	else
	{
		debugger = it->second;
	}

	debugger->SetCurrentState(L);

	return debugger;
}

std::shared_ptr<Debugger> BluePrintDebuggerManager::RemoveDebugger(lua_State *L)
{
	std::lock_guard<std::mutex> lock(debuggerMtx);
	auto identify = GetUniqueIdentify(L);
	auto it = debuggers.find(identify);
	if (it != debuggers.end())
	{
		auto debugger = it->second;
		debuggers.erase(it);
		return debugger;
	}
	return nullptr;
}

std::vector<std::shared_ptr<Debugger>> BluePrintDebuggerManager::GetDebuggers()
{
	std::lock_guard<std::mutex> lock(debuggerMtx);
	std::vector<std::shared_ptr<Debugger>> debuggerVector;
	for (auto it : debuggers)
	{
		debuggerVector.push_back(it.second);
	}
	return debuggerVector;
}

void BluePrintDebuggerManager::RemoveAllDebugger()
{
	std::lock_guard<std::mutex> lock(debuggerMtx);
	debuggers.clear();
}

std::shared_ptr<Debugger> BluePrintDebuggerManager::GetBreakedpoint()
{
	std::lock_guard<std::mutex> lock(breakDebuggerMtx);
	return breakedDebugger;
}

void BluePrintDebuggerManager::SetBreakedDebugger(std::shared_ptr<Debugger> debugger)
{
	std::lock_guard<std::mutex> lock(breakDebuggerMtx);
	breakedDebugger = debugger;
}

bool BluePrintDebuggerManager::IsDebuggerEmpty()
{
	std::lock_guard<std::mutex> lock(debuggerMtx);
	return debuggers.empty();
}

void BluePrintDebuggerManager::AddBreakpoint(std::shared_ptr<BreakPoint> breakpoint)
{
	std::lock_guard<std::mutex> lock(breakpointsMtx);
	bool isAdd = false;
	for (std::shared_ptr<BreakPoint> &bp : breakpoints)
	{
		if (bp->line == breakpoint->line && CompareIgnoreCase(bp->file, breakpoint->file) == 0)
		{
			bp = breakpoint;
			isAdd = true;
		}
	}

	if (!isAdd)
	{
		breakpoints.push_back(breakpoint);
	}

	RefreshLineSet();
}

std::vector<std::shared_ptr<BreakPoint>> BluePrintDebuggerManager::GetBreakpoints()
{
	std::lock_guard<std::mutex> lock(breakpointsMtx);
	return breakpoints;
}

void BluePrintDebuggerManager::RemoveBreakpoint(const std::string &file, int line)
{
	std::lock_guard<std::mutex> lock(breakpointsMtx);
	auto it = breakpoints.begin();
	while (it != breakpoints.end())
	{
		const auto bp = *it;
		if (bp->line == line && CompareIgnoreCase(bp->file, file) == 0)
		{
			breakpoints.erase(it);
			break;
		}
		++it;
	}
	RefreshLineSet();
}

void BluePrintDebuggerManager::RemoveAllBreakPoints()
{
	std::lock_guard<std::mutex> lock(breakpointsMtx);
	breakpoints.clear();
	lineSet.clear();
}

void BluePrintDebuggerManager::RefreshLineSet()
{
	lineSet.clear();
	for (auto bp : breakpoints)
	{
		lineSet.insert(bp->line);
	}
}

std::set<int> BluePrintDebuggerManager::GetLineSet()
{
	std::lock_guard<std::mutex> lock(breakpointsMtx);
	return lineSet;
}

void BluePrintDebuggerManager::HandleBreak(lua_State *L)
{
	auto debugger = GetDebugger(L);

	if (debugger)
	{
		debugger->SetCurrentState(L);
	}
	else
	{
		debugger = AddDebugger(L);
	}

	SetBreakedDebugger(debugger);

	debugger->HandleBreak();
}

void BluePrintDebuggerManager::DoAction(DebugAction action)
{
	auto debugger = GetBreakedpoint();
	if (debugger)
	{
		debugger->DoAction(action);
	}
}

void BluePrintDebuggerManager::SetValueByCommand(const int currentFrameId, const std::string commandData)
{
	auto debugger = GetBreakedpoint();
	if (debugger)
	{
		debugger->SetValueByCommand(currentFrameId, commandData);
	}
}

void BluePrintDebuggerManager::Eval(std::shared_ptr<EvalContext> ctx)
{
	auto debugger = GetBreakedpoint();
	if (debugger)
	{
		debugger->Eval(ctx, false);
	}
}

void BluePrintDebuggerManager::OnDisconnect()
{
	SetRunning(false);
	std::lock_guard<std::mutex> lock(debuggerMtx);
	for (auto it : debuggers)
	{
		it.second->Stop();
	}
}

void BluePrintDebuggerManager::SetRunning(bool value)
{
	isRunning = value;
	if (isRunning)
	{
		for (auto debugger : GetDebuggers())
		{
			debugger->Start();
		}
	}
}

bool BluePrintDebuggerManager::IsRunning()
{
	return isRunning;
}

BluePrintDebuggerManager::UniqueIdentifyType BluePrintDebuggerManager::GetUniqueIdentify(lua_State *L)
{
	if (luaVersion == LuaVersion::LUA_JIT)
	{
		// 我们认为luajit只会有一个debugger
		return 0;
	}
	else
	{
		return reinterpret_cast<UniqueIdentifyType>(GetMainState(L));
	}
}
