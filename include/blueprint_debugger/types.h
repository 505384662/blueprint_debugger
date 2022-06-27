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
#pragma once

#include <string>
#include <vector>
#include <memory>

enum class DebugAction
{
	Break,
	Continue,
	StepOver,
	StepIn,
	StepOut,
	Stop,
};


enum ELuaVariableType
{
	None,//未知
	Local,//本地值
	UpValue,//上值
	
};

class BreakPoint
{
public:
	std::string file;
	std::string condition;
	std::string hitCondition;
	std::string logMessage;
	// std::vector<std::string> pathParts;
	int hitCount = 0;
	int line = 0;
};



class Variable
{
public:
	Variable();
	~Variable();

	std::string parentName = "";
	std::string name;
	int nameType = 0;
	std::string value;
	int valueType = 0;
	std::string valueTypeName;
	std::vector<std::shared_ptr<Variable>> children;
	int cacheId = 0;
	int nVar = 0; //值得索引位置
	int frameId = 0;
	ELuaVariableType luaType = ELuaVariableType::None;

	std::shared_ptr<Variable> Clone();
	std::shared_ptr<Variable> CreateChildNode();
};

class Stack
{
public:
	std::string file;
	std::string functionName;
	int frameId = 0;
	int line = 0;
	std::vector<std::shared_ptr<Variable>> localVariables;
	std::vector<std::shared_ptr<Variable>> upvalueVariables;
public:
	Stack();
	~Stack();
	std::shared_ptr<Variable> CreateVariable();
};

class EvalContext
{
public:
	EvalContext()
		: result(std::make_shared<Variable>())
	{
	}

	std::string expr;
	std::string error;
	int seq = 0;
	int stackLevel = 0;
	int depth = 0;
	int cacheId = 0;
	std::shared_ptr<Variable> result;
	bool success = false;
};

class LogMessageReplaceExpress
{
public:
	LogMessageReplaceExpress(std::string&& expr, std::size_t startIndex, std::size_t endIndex, bool needEval)
		: Expr(expr),
		  StartIndex(startIndex),
		  EndIndex(endIndex),
		  NeedEval(needEval)
	{
	}

	std::string Expr;
	std::size_t StartIndex;
	std::size_t EndIndex;
	bool NeedEval;
};
