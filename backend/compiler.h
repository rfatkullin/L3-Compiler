#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <map>
#include <stack>
#include <string>
#include <string.h>
#include "node.h"
#include "variable.h"
#include "utility.h"
#include "code_generator.h"

namespace L3Compiler
{
    #define CHECK_TRUE(param) if (!(param)) return false;	    

	class Compiler
	{
	public :
        Compiler(SubsDefNode* program, const char* outputFilePath);
        ~Compiler();
		void Run();

	private :
		SubsDefNode* _program;
        CodeGenerator* _codeGen;
        std::map<const char*, Variable, StrCmp> _scopeVars;
        std::stack<TypeNode> _stackValuesTypes;
        int _blockArgsCount;
        int _blockLocalsCount;		

        const char* GetSubName(SubDefNode* node);
        std::string GetString(TypeNode* type);
        void EnterTheBlock();
        bool IsExistsMainFunc();
		bool StaticTest();
        bool FindVariable(const char* ident, Variable& var);
        bool TypeMatch(const TypeNode& aT, const TypeNode& bT);

        bool ProcessSubsDef();
        bool ProcessSubDefNode(SubDefNode* node);
        bool ProcessFuncDef(FuncDefNode* node);
        bool ProcessProcDef(ProcDefNode* node);
        bool ProcessSignature(SigNode* node);
        bool ProcessParamsDef(ParamsDefNode* node);
        bool ProcessVarDefs(VarsDefNode* node);
        bool ProcessStatements(StatementsNode* node);
        bool ProcessAssign(AssignNode* node);
        bool ProcessFuncCall(FuncCallNode* node);
        bool ProcessIfStatement(IfNode* node);
        bool ProcessWhileStatement(WhileDoNode* node);
        bool ProcessForStatement(ForNode* node);
        bool ProcessRepeatStatement(RepeatNode* node);
        bool ProcessCheckStatement(CheckNode* node);
        bool ProcessNewArrAssign(Variable var, NewArrNode* node);
        bool ProcessExprAssign(Variable var, ExprNode* node);

	};

} // L3Compiler namespace

#endif
