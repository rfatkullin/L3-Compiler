#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <map>
#include <list>
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
	#define CHECK_TRUE(param) if (!(param)) return false;
	#define ON_FALSE_ERR(param, errId) {if (!(param)) PRINT_ERR_RETURN(errId)}
	#define ON_TRUE_ERR(param, errId) {if (param) PRINT_ERR_RETURN(errId)}
	#define PRINT_ERR_RETURN(errId) {printf("[Error]: %s\n", Msg::ErrorsStrList[errId]); return false;}

	class Compiler
	{
	public :
        Compiler(SubsDefNode* program, const char* outputFilePath);
        ~Compiler();
		bool Run();

	private :		

		typedef std::map<const char*, std::pair<TypeNode*, SigNode::SubParams*>, StrCmp> SubsMap;

		SubsDefNode* _program;
        CodeGenerator* _codeGen;

        std::map<const char*, Variable, StrCmp> _scopeVars;		
        std::stack<TypeNode> _stackValuesTypes;
        int _blockArgsCount;
		int _blockLocalsCount;

		SubsMap _subs;

		const char* _currSubName;

		std::map<int, Variable> _scopeTmpVars;
		std::set<int> _allScopeTmpVars;
		std::set<int> _freeScopeTmpVars;

		std::string GetString(TypeNode* type);
        void EnterTheBlock();
		bool CheckMainFunc();
		bool StaticTest();
        bool FindVariable(const char* ident, Variable& var);

		bool SubsDefProcess();
		bool SubDefNodeProcess(SubDefNode* node);
		bool FuncDefProcess(FuncDefNode* node);
		bool ProcDefProcess(ProcDefNode* node);
		bool SignatureProcess(TypeNode* type, SigNode* node);
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
		bool ExprAssignProcess(Variable var, ExprNode* node);
		bool ExprProcess(ExprNode* node);
		bool PrintStatementProcess(PrintNode* node);
		bool NewArrProcess(NewArrNode* node);
		bool GetArrElProcess(ArrElNode* node);
		bool ArrElProcess(ArrElNode* node);

		bool IsVoidType(const TypeNode& type);
		bool IsIntType(const TypeNode& type);
		bool IsCharType(const TypeNode& type);
		bool IsBoolType(const TypeNode& type);
		bool IsCharOrIntType(const TypeNode& type);

		bool CompareOpArgCheck(const TypeNode& type1, const TypeNode& type2);
		bool AddOpArgCheck(const TypeNode& type1, const TypeNode& type2);

		bool AddScopeVar(const char* ident, int id, TypeNode* type, bool isArg);
		bool AddScopeVar(const char* ident, const Variable& var);

		bool TypeMatch(const TypeNode& aT, const TypeNode& bT);

		Variable GetFreeTmpVar(const TypeNode& type);
		void RetrieveTmpVar(int num);
	};

} // L3Compiler namespace

#endif
