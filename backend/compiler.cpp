#include <string.h>
#include "compiler.h"
#include "parser.tab.h"

namespace L3Compiler
{
	Compiler :: Compiler(SubsDefNode* program, FILE* out)
		: 	_program(program),
			_out(out)
	{
	}

	void Compiler :: Run()
	{
		if (!StaticTest())
		{
			printf("Static test failed!\n");
			return;
		}

		fprintf(_out, ".assembly AzazaAssembly {}\n");
		fprintf(_out, ".assembly extern mscorlib {}\n\n");

		ProcessSubsDef();
	}

	std::string Compiler :: GetString(TypeNode* node)
	{
		std::string str = "";

		switch (node->type)
		{
			case INT_TYPE:
				str += "int32";
				break;
			case BOOL_TYPE:
				str += "int8";
				break;
			case CHAR_TYPE:
				str += "int8";
				break;
		}

		for (int i = 0; i < node->dimen; ++i)
			str += "[]";

		return str;
	}

	bool Compiler :: StaticTest()
	{
		if (!_program)
		{
			printf("Empty source\n");
			return false;
		}

		if (!IsExistsMainFunc())
		{
			printf("Can't find main func\n");
			return false;
		}

		return true;
	}

	bool Compiler :: IsExistsMainFunc()
	{
		SubsDefNode* node = _program;

		while (node)
		{
			if (!strcmp("main", GetSubName(node->def)))
				return true;

			node = node->tail;
		}

		return false;
	}

	const char* Compiler :: GetSubName(SubDefNode* node)
	{
		switch (node->tag)
		{
			case FUNC :
				return node->func->signature->ident;
				break;
			case PROC :
				return node->proc->signature->ident;
				break;
			default :
				return "";
				break;
		}
	}

	void Compiler :: ProcessSubsDef()
	{
		SubsDefNode* node = _program;

		while (node)
		{
			ProcessSubDefNode(node->def);
			node = node->tail;
		};
	}

	void Compiler :: ProcessSubDefNode(SubDefNode* node)
	{
		fprintf(_out, ".method static ");

		switch (node->tag)
		{
			case FUNC :
				ProcessFuncDef(node->func);
				break;
			case PROC :
				ProcessProcDef(node->proc);
				break;
			default :
				printf("Unexpected structure with tag = %d!\n", node->tag);
				break;
		}
	}

	void Compiler :: ProcessFuncDef(FuncDefNode* node)
	{
		_scopeVariables.clear();

		fprintf(_out, "%s ", GetString(node->type).c_str());

		ProcessSignature(node->signature);

		fprintf(_out, "{\n");

		ProcessStatements(node->statements);

		fprintf(_out, "}\n\n");
	}

	void Compiler :: ProcessProcDef(ProcDefNode* node)
	{
		_scopeVariables.clear();

		fprintf(_out, "void ");

		ProcessSignature(node->signature);

		fprintf(_out, "{\n");

		if (!strcmp(node->signature->ident, "main"))
		{
			fprintf(_out, ".entrypoint\n");
		}

		ProcessStatements(node->statements);

		fprintf(_out, "}\n\n");
	}

	void Compiler :: ProcessSignature(SigNode* node)
	{
		fprintf(_out, "%s", node->ident);

		ProcessParamsDef(node->params_def);
	}

	void Compiler :: ProcessParamsDef(ParamsDefNode* params)
	{
		fprintf(_out, "(");

		while (params)
		{
			IdentsNode* idents = params->params_sec->idents;
			std::string typeStr = GetString(params->params_sec->type);

			while (idents)
			{
				if (_scopeVariables.find(idents->ident) != _scopeVariables.end())
				{
					printf("[Error]: conflicting in declaration function args!\n");
				}

				_scopeVariables.insert(std::pair<char*, Variable>(idents->ident, Variable(_scopeVariables.size(), true, params->params_sec->type)));

				if ((idents->tail != NULL) || (params->tail != NULL))
					fprintf(_out, "%s, ", typeStr.c_str());
				else
					fprintf(_out, "%s", typeStr.c_str());

				idents = idents->tail;
			};

			params = params->tail;
		}

		fprintf(_out, ")\n");
	}

	void Compiler :: ProcessStatements(StatementsNode* node)
	{
		//fprintf(_out, "statement1;\nstatement2\n");
	}

} //L3Compiler namespace