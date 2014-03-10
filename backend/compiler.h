#include <map>
#include <string>
#include <string.h>
#include "node.h"

namespace L3Compiler
{
	struct Variable
	{
		Variable(int id, bool isArg, TypeNode* type)
			:	_id(id),
				_isArg(isArg),
				_type(type)
		{}

		int 		_id;
		bool		_isArg;
		TypeNode* 	_type;
	};

	struct cmp_str
	{
	   bool operator()(const char* a, const char* b)
	   {
	      return strcmp(a, b) < 0;
	   }
	};

	class Compiler
	{
	public :
		Compiler(SubsDefNode* program, FILE* out);
		void Run();

	private :
		SubsDefNode* _program;
		FILE* _out;
		std::map<const char*, Variable, cmp_str> _scopeVariables;

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
		std::string GetString(TypeNode* type);
	};

} // L3Compiler namespace