#include <string.h>
#include "compiler.h"
#include "parser.tab.h"

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

	fprintf(_out, ".assembly Azaza-assembly {}\n");
	fprintf(_out, ".assembly extern mscorlib {}\n");

	ProcessSubsDef();
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
	fprintf(_out, "type ");

	ProcessSignature(node->signature);

	fprintf(_out, "{\n");

	ProcessStatements(node->statements);

	fprintf(_out, "}\n\n");
}

void Compiler :: ProcessProcDef(ProcDefNode* node)
{
	fprintf(_out, "void ");

	ProcessSignature(node->signature);

	fprintf(_out, "{\n");

	ProcessStatements(node->statements);

	fprintf(_out, "}\n\n");
}

void Compiler :: ProcessSignature(SigNode* node)
{
	fprintf(_out, "%s", node->ident);

	ProcessParamsDef(node->params_def);
}

void Compiler :: ProcessParamsDef(ParamsDefNode* node)
{
	fprintf(_out, "(");

	fprintf(_out, "params");

	fprintf(_out, ")\n");
}

void Compiler :: ProcessStatements(StatementsNode* node)
{
	fprintf(_out, "statement1;\nstatement2\n");
}


