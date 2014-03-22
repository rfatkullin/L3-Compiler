#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <map>
#include <stack>
#include <string>
#include <set>
#include <string.h>
#include "node.h"
#include "variable.h"
#include "utility.h"
#include "code_generator.h"
#include "msg.h"

namespace L3Compiler
{
	#define CHECK_SUCCESS(param) if (!(param)) return false;
	#define CHECK_SUCCESS_PRINT_ERR(param, errId) {if (!(param)) PRINT_ERR_RETURN(errId)}
	#define PRINT_ERR_RETURN(errId) {printf("[Error]: %s\n", Msg::ErrorsStrList[errId]); return false;}

	class Compiler
	{
	public :
        Compiler(SubsDefNode* program, const char* outputFilePath);
        ~Compiler();
		bool Run();

	private :		

		SubsDefNode* _program;
        CodeGenerator* _codeGen;
        std::map<const char*, Variable, StrCmp> _scopeVars;		
        std::stack<TypeNode> _stackValuesTypes;
        int _blockArgsCount;
		int _blockLocalsCount;

		std::map<int, Variable> _scopeTmpVars;
		std::set<int> _allScopeTmpVars;
		std::set<int> _freeScopeTmpVars;

		const char* GetSubName(SubDefNode* node);
        std::string GetString(TypeNode* type);
        void EnterTheBlock();
        bool IsExistsMainFunc();
		bool StaticTest();
        bool FindVariable(const char* ident, Variable& var);

		bool SubsDefProcess();
		bool SubDefNodeProcess(SubDefNode* node);
		bool FuncDefProcess(FuncDefNode* node);
		bool ProcDefProcess(ProcDefNode* node);
		bool SignatureProcess(SigNode* node);
		bool ParamsDefProcess(ParamsDefNode* node);
		bool VarDefsProcess(VarsDefNode* node);
		bool VarDefProcess(VarNode* var, TypeNode* type);
		bool StatementsProcess(StatementsNode* node);
		bool AssignProcess(AssignNode* node);
		bool FuncCallProcess(FuncCallNode* node);
		bool IfStatementProcess(IfNode* node);
		bool WhileStatementProcess(WhileDoNode* node);
		bool ForStatementProcess(ForNode* node);
		bool RepeatStatementProcess(RepeatNode* node);
		bool CheckStatementProcess(CheckNode* node);
		bool NewArrAssignProcess(Variable var, NewArrNode* node);
		bool ExprAssignProcess(Variable var, ExprNode* node);
		bool ExprProcess(ExprNode* node);
		bool PrintStatementProcess(PrintNode* node);

		bool IsIntType(const TypeNode& type);
		bool IsCharType(const TypeNode& type);
		bool IsBoolType(const TypeNode& type);
		bool IsCharOrIntType(const TypeNode& type);

		bool AddScopeVar(const char* ident, int id, TypeNode* type, bool isArg);
		bool AddScopeVar(const char* ident, const Variable& var);

		bool TypeMatch(const TypeNode& aT, const TypeNode& bT);

		Variable GetFreeTmpVar(const TypeNode& type);
		void RetrieveTmpVar(int num);
	};

} // L3Compiler namespace

#endif
