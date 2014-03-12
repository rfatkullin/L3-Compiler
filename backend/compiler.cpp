#include <string.h>
#include "compiler.h"
#include "parser.tab.h"

namespace L3Compiler
{
    Compiler :: Compiler(SubsDefNode* program, const char* outputFilePath)
        : 	_program(program)
	{
        _codeGen = new CodeGenerator(outputFilePath);
	}

    Compiler :: ~Compiler()
    {
        delete _codeGen;
    }

	void Compiler :: Run()
	{
		if (!StaticTest())
		{
			printf("Static test failed!\n");
			return;
		}		

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

	bool Compiler :: ProcessSubsDef()
	{
		SubsDefNode* node = _program;

		while (node)
		{
			CHECK_TRUE(ProcessSubDefNode(node->def));
			node = node->tail;
		};

		return true;
	}

	bool Compiler :: ProcessSubDefNode(SubDefNode* node)
    {
		switch (node->tag)
		{
			case FUNC :
				CHECK_TRUE(ProcessFuncDef(node->func));
				break;
			case PROC :
				CHECK_TRUE(ProcessProcDef(node->proc));
				break;
			default :
				printf("Unexpected structure with tag = %d!\n", node->tag);
				return false;
				break;
		}

		return true;
	}

	bool Compiler :: ProcessFuncDef(FuncDefNode* node)
	{
        EnterTheBlock();

        _codeGen->SubDef(node->type);

        CHECK_TRUE(ProcessSignature(node->signature));

        _codeGen->BlockStart();

        CHECK_TRUE(ProcessStatements(node->statements));

        _codeGen->BlockEnd(_scopeVars);

		return true;
	}

	bool Compiler :: ProcessProcDef(ProcDefNode* node)
	{
        EnterTheBlock();

        _codeGen->SubDef();

        CHECK_TRUE(ProcessSignature(node->signature));

        _codeGen->BlockStart();

        CHECK_TRUE(ProcessStatements(node->statements));

        _codeGen->BlockEnd(_scopeVars);

		return true;
	}

	bool Compiler :: ProcessSignature(SigNode* node)
	{
        _codeGen->SubName(node->ident);

        CHECK_TRUE(ProcessParamsDef(node->params_def));

		return true;
	}

    void Compiler :: EnterTheBlock()
    {
        _scopeVars.clear();
        _stackValuesTypes = std::stack<TypeNode>();
        _blockArgsCount = 0;
        _blockLocalsCount = 0;        
    }

	bool Compiler :: ProcessParamsDef(ParamsDefNode* params)
	{
        _codeGen->SignatureStart();

		while (params)
		{
			IdentsNode* idents = params->params_sec->idents;
            std::string typeStr = CodeGenerator::TypeToString(params->params_sec->type);

			while (idents)
			{
                if (_scopeVars.find(idents->ident) != _scopeVars.end())
				{
					printf("[Error]: conflicting in declaration function args!\n");
					return false;
				}

                _scopeVars.insert(std::pair<char*, Variable>(idents->ident, Variable(_blockArgsCount++, params->params_sec->type, true)));

                bool isContinious = ((idents->tail != NULL) || (params->tail != NULL));
                _codeGen->Def(typeStr, isContinious);

				idents = idents->tail;
			};

			params = params->tail;
		}

        _codeGen->SignatureEnd();

		return true;
	}

    bool Compiler :: ProcessStatements(StatementsNode* node)
	{
        while (node)
		{
            StatementNode* stm = node->statement;

            switch (stm->tag)
			{
				case VARS_DEF :
                    CHECK_TRUE(ProcessVarDefs(stm->vars_def));
					break;

				case ASSIGN :
                    CHECK_TRUE(ProcessAssign(stm->assign));
					break;

				case FUNC_CALL :
                    CHECK_TRUE(ProcessFuncCall(stm->func_call));
					break;

				case IF :
                    CHECK_TRUE(ProcessIfStatement(stm->if_statement));
					break;

				case WHILE :
                    CHECK_TRUE(ProcessWhileStatement(stm->while_do));
					break;

				case FOR :
                    CHECK_TRUE(ProcessForStatement(stm->for_statement));
					break;

				case REPEAT :
                    CHECK_TRUE(ProcessRepeatStatement(stm->repeat));
					break;

				case CHECK :
                    CHECK_TRUE(ProcessCheckStatement(stm->check));
					break;

				default :
					printf("Unexpected operator!\n");
					return false;
			}

            node = node->tail;
		}

		return true;
	}

	bool Compiler :: ProcessVarDefs(VarsDefNode* node)
	{
		VarsNode* vars = node->vars;

		while (vars)
		{
            Variable localVariable(_blockLocalsCount++, node->type, false);
            if (_scopeVars.find(vars->var->ident) != _scopeVars.end())
			{
				printf("[Error]: conflicting in declaration function locals!\n");
				return false;
			}

            _scopeVars.insert(std::pair<char*, Variable>(vars->var->ident, localVariable));

			switch (vars->var->tag)
			{
				case IDENT :
					//Nothing to do - only add to locals map
					break;

				case EXPR :
					CHECK_TRUE(ProcessExprAssign(localVariable, vars->var->expr));
					break;

				case NEW_ARR :
                    CHECK_TRUE(ProcessNewArrAssign(localVariable, vars->var->new_arr));
					break;

				default :
					printf("Expected identificator/expr/new arr!\n");
					return false;
			}

			vars = vars->tail;
		}

		return true;
	}

    bool Compiler :: ProcessExprAssign(Variable var, ExprNode* node)
    {
        Variable exprVar;

        while (node->op == UNARY)
            node = node->un.expr;

        switch(node->op)
        {
        case NUMBER :
            _stackValuesTypes.push(TypeNode(INT_TYPE, 0));
            _codeGen->LoadIntConst(node->un.num);
            break;

        case TT :
            _stackValuesTypes.push(TypeNode(BOOL_TYPE, 0));
            _codeGen->LoadBoolConst(true);
            break;

        case FF :
            _stackValuesTypes.push(TypeNode(BOOL_TYPE, 0));
            _codeGen->LoadBoolConst(false);
            break;

        case CHAR :
            _stackValuesTypes.push(TypeNode(CHAR_TYPE, 0));
            _codeGen->LoadIntConst(node->un.ch);
            break;

        case STR :
            _stackValuesTypes.push(TypeNode(CHAR_TYPE, 1));
            _codeGen->LoadStr(node->un.str);
            break;

        case IDENT :
            CHECK_TRUE(FindVariable(node->un.ident, exprVar));
            _stackValuesTypes.push(*exprVar._type);
            _codeGen->LoadVariable(exprVar);
            break;

        default :
            printf("[Error]: Unknown expr!\n");
            return false;
        }

        if (!TypeMatch(_stackValuesTypes.top(), *var._type))
        {
            printf("[Error]: type mismatch!\n");
            return false;
        }

        _stackValuesTypes.pop();
        _codeGen->SaveFromStack(var);

        return true;
    }

    bool Compiler :: ProcessNewArrAssign(Variable var, NewArrNode* node)
    {
        return true;
    }


    bool Compiler :: ProcessAssign(AssignNode* node)
    {        
        return true;
    }

    bool Compiler :: ProcessFuncCall(FuncCallNode* node)
    {
        return true;
    }

    bool Compiler :: ProcessIfStatement(IfNode* node)
    {
        return true;
    }

    bool Compiler :: ProcessWhileStatement(WhileDoNode* node)
    {
        return true;
    }

    bool Compiler :: ProcessForStatement(ForNode* node)
    {
        return true;
    }

    bool Compiler :: ProcessRepeatStatement(RepeatNode* node)
    {
        return true;
    }

    bool Compiler :: ProcessCheckStatement(CheckNode* node)
    {
        return true;
    }

    bool Compiler :: TypeMatch(const TypeNode& aT, const TypeNode& bT)
    {
        return (aT.type == bT.type) && (aT.dimen == bT.dimen);
    }

    bool Compiler :: FindVariable(const char* ident, Variable& var)
    {
        std::map<const char*, Variable, StrCmp>::iterator it = _scopeVars.find(ident);

        if (it == _scopeVars.end())
            return false;

        var = (*it).second;

        return true;
    }


} //L3Compiler namespace
