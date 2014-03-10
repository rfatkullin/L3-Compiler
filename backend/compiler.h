#include "node.h"

class Compiler
{
public :
	Compiler(SubsDefNode* program, FILE* out);
	void Run();

private :
	SubsDefNode* _program;
	FILE* _out;

	bool StaticTest();

	void ProcessSubsDef();
	void ProcessSubDefNode(SubDefNode* node);
	void ProcessFuncDef(FuncDefNode* node);
	void ProcessProcDef(ProcDefNode* node);
	const char* GetSubName(SubDefNode* node);
	void ProcessSignature(SigNode* node);
	void ProcessParamsDef(ParamsDefNode* node);
	void ProcessStatements(StatementsNode* node);
	bool IsExistsMainFunc();
};