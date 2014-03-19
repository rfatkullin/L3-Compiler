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

	bool Compiler :: Run()
	{
		if (!StaticTest())
		{
			printf("Static test failed!\n");
			return false;
		}		

		return SubsDefProcess();
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

	bool Compiler :: SubsDefProcess()
	{
		SubsDefNode* node = _program;

		while (node)
		{
			CHECK_SUCCESS(SubDefNodeProcess(node->def));
			node = node->tail;
		};

		return true;
	}

	bool Compiler :: SubDefNodeProcess(SubDefNode* node)
	{
		switch (node->tag)
		{
			case FUNC :
				CHECK_SUCCESS(FuncDefProcess(node->func));
				break;
			case PROC :
				CHECK_SUCCESS(ProcDefProcess(node->proc));
				break;
			default :
				printf("Unexpected structure with tag = %d!\n", node->tag);
				return false;
				break;
		}

		return true;
	}

	bool Compiler :: FuncDefProcess(FuncDefNode* node)
	{
        EnterTheBlock();

        _codeGen->SubDef(node->type);

		CHECK_SUCCESS(SignatureProcess(node->signature));

        _codeGen->BlockStart();

		CHECK_SUCCESS(StatementsProcess(node->statements));

        _codeGen->BlockEnd(_scopeVars);

		return true;
	}

	bool Compiler :: ProcDefProcess(ProcDefNode* node)
	{
        EnterTheBlock();

        _codeGen->SubDef();

		CHECK_SUCCESS(SignatureProcess(node->signature));

        _codeGen->BlockStart();

		CHECK_SUCCESS(StatementsProcess(node->statements));

        _codeGen->BlockEnd(_scopeVars);

		return true;
	}

	bool Compiler :: SignatureProcess(SigNode* node)
	{
        _codeGen->SubName(node->ident);

		CHECK_SUCCESS(ParamsDefProcess(node->params_def));

		return true;
	}

    void Compiler :: EnterTheBlock()
    {
        _scopeVars.clear();
        _stackValuesTypes = std::stack<TypeNode>();
        _blockArgsCount = 0;
        _blockLocalsCount = 0;        

		for (std::map<int, Variable>::iterator it = _scopeTmpVars.begin(); it != _scopeTmpVars.end(); ++it)
		{
			delete it->second._type;
		}

		_scopeTmpVars.clear();
		_allScopeTmpVars.clear();
		_freeScopeTmpVars.clear();
    }

	bool Compiler :: ParamsDefProcess(ParamsDefNode* params)
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

	bool Compiler :: StatementsProcess(StatementsNode* node)
	{
        while (node)
		{
            StatementNode* stm = node->statement;

            switch (stm->tag)
			{
				case VARS_DEF :
					CHECK_SUCCESS(VarDefsProcess(stm->vars_def));
					break;

				case ASSIGN :
					CHECK_SUCCESS(AssignProcess(stm->assign));
					break;

				case FUNC_CALL :
					CHECK_SUCCESS(FuncCallProcess(stm->func_call));
					break;

				case IF :
					CHECK_SUCCESS(IfStatementProcess(stm->if_statement));
					break;

				case WHILE :
					CHECK_SUCCESS(WhileStatementProcess(stm->while_do));
					break;

				case FOR :
					CHECK_SUCCESS(ForStatementProcess(stm->for_statement));
					break;

				case REPEAT :
					CHECK_SUCCESS(RepeatStatementProcess(stm->repeat));
					break;

				case CHECK :
					CHECK_SUCCESS(CheckStatementProcess(stm->check));
					break;

				case PRINT :
					CHECK_SUCCESS(PrintStatementProcess(stm->print));
					break;

				default :
					printf("Unexpected operator!\n");
					return false;
			}

            node = node->tail;
		}

		return true;
	}

	bool Compiler :: VarDefsProcess(VarsDefNode* node)
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
					CHECK_SUCCESS(ExprAssignProcess(localVariable, vars->var->expr));
					break;

				case NEW_ARR :
					CHECK_SUCCESS(NewArrAssignProcess(localVariable, vars->var->new_arr));
					break;

				default :
					printf("Expected identificator/expr/new arr!\n");
					return false;
			}

			vars = vars->tail;
		}

		return true;
	}

	bool Compiler :: ExprAssignProcess(Variable var, ExprNode* exprNode)
    {
		CHECK_SUCCESS(ExprProcess(exprNode));

		if (!TypeMatch(_stackValuesTypes.top(), *var._type))
        {
            printf("[Error]: type mismatch!\n");
            return false;
        }

        _stackValuesTypes.pop();
        _codeGen->SaveFromStack(var);

        return true;
    }

	bool Compiler :: ExprProcess(ExprNode* node)
	{		
		TypeNode type1(INT_TYPE, 0);
		Variable exprVar;
		bool isUnary = true;

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
			CHECK_SUCCESS_PRINT_ERR(FindVariable(node->un.ident, exprVar), Msg::VariableNotDiclared);
			_stackValuesTypes.push(*exprVar._type);
			_codeGen->LoadVariable(exprVar);
			break;

		case EXCL :
			CHECK_SUCCESS(ExprProcess(node->un.expr));
			CHECK_SUCCESS(IsBoolType(_stackValuesTypes.top()));
			_codeGen->NotOperator();
			break;

		case UMINUS :
			CHECK_SUCCESS(ExprProcess(node->un.expr));

			type1 = _stackValuesTypes.top();
			_stackValuesTypes.pop();

			CHECK_SUCCESS(IsBoolType(type1) || IsIntType(type1));

			_codeGen->NegOperator();
			_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			break;

		case ARR_EL :
			//To Fix
			break;

		default :
			isUnary = false;
			break;
		}

		if (isUnary)
			return true;

		CHECK_SUCCESS(ExprProcess(node->bin.left_expr));
		CHECK_SUCCESS(ExprProcess(node->bin.right_expr));

		TypeNode type2 = _stackValuesTypes.top();
		_stackValuesTypes.pop();
		type1 = _stackValuesTypes.top();
		int tmpLocVarInd = -1;

		switch (node->op)
		{
		case LOG_OR  :
		case LOG_AND :
		case LOG_CAP :

			if (!IsBoolType(type1) || !IsBoolType(type2))
			{
				printf("[Error]: Operands of boolean operator must be boolean!\n");
				return false;
			}

			switch (node->op)
			{
				case LOG_AND :
					_codeGen->LogAndOperator();
					break;

				case LOG_OR  :
					_codeGen->LogOrOperator();
					break;

				case LOG_CAP :
					_codeGen->LogXorOperator();
					break;
			}
			break;

		case EQ		:
		case NOT_EQ :

			if (!TypeMatch(type1, type2) &&
				(!IsCharOrIntType(type1) || !IsCharOrIntType(type2)))
			{
				printf("[Error]: type mismatch!\n");
				return false;
			}

			if (node->op == EQ)
				_codeGen->EqOperator();
			else
				_codeGen->NotEqOperator();

			break;

		case LSS	:
		case GTR	:
		case LSS_EQ :
		case GTR_EQ :

			if (!IsCharOrIntType(type1) || !IsCharOrIntType(type2))
			{
				printf("[Error]: type mismatch!\n");
				return false;
			}

			switch (node->op)
			{
				case LSS :
					_codeGen->LssOperator();
					break;

				case GTR :
					_codeGen->GtrOperator();
					break;

				case LSS_EQ :
					_codeGen->LssEqOperator();
					break;

				case GTR_EQ :
					_codeGen->GtrEqOperator();
					break;

				default :
					break;
			}

			break;

		case PLUS	:
			_stackValuesTypes.pop();
			if (IsIntType(type1) && IsIntType(type2))
			{
				_codeGen->Add();
				_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			}
			else if (IsIntType(type1)  && IsCharType(type2) ||
					 IsCharType(type1) && IsIntType(type2))
			{
				_codeGen->Add();
				_stackValuesTypes.push(TypeNode(CHAR_TYPE, 0));
			}
			else
			{
				printf("[Error]: type mismatch!\n");
				return false;
			}
			break;

		case MINUS	:
			_stackValuesTypes.pop();

			if (IsIntType(type1)  && IsIntType(type2) ||
				IsCharType(type1) && IsCharType(type2))
			{
				_codeGen->Sub();
				_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			}
			else if (IsCharType(type1) && IsIntType(type2))
			{
				_codeGen->Sub();
				_stackValuesTypes.push(TypeNode(CHAR_TYPE, 0));
			}
			else
			{
				printf("[Error]: type mismatch!\n");
				return false;
			}
			break;

		//MULTIPLY
		case DIVIDE :
		case MOD	:
		case CAP	:

			if (!IsIntType(type1) || !IsIntType(type2))
			{
				printf("[Error]: Operands of ^,/,%% operators must be boolean!\n");
				return false;
			}

			switch (node->op)
			{
				case DIVIDE :
					_codeGen->DivOperator();
					break;
				case MOD :
					_codeGen->ModOperator();
					break;
				case CAP :
					tmpLocVarInd = GetFreeTmpVar(TypeNode(INT_TYPE, 0));
					_codeGen->PowOperator(tmpLocVarInd);
					RetrieveTmpVar(tmpLocVarInd);
					break;

				default :
					break;
			}

			break;

		default :
			printf("[Error]: Unknown expr!\n");
			return false;
		}

		return true;
	}

	bool Compiler :: NewArrAssignProcess(Variable var, NewArrNode* node)
    {
        return true;
    }


	bool Compiler :: AssignProcess(AssignNode* node)
    {        
		std::map<const char*, Variable, StrCmp>::iterator varIt;

		CHECK_SUCCESS(ExprProcess(node->expr));

		switch (node->left->tag)
		{
		case IDENT :
			varIt = _scopeVars.find(node->left->ident);

			if (varIt == _scopeVars.end())
				PRINT_ERR_RETURN(Msg::VariableNotDiclared);

			if (!TypeMatch(*varIt->second._type, _stackValuesTypes.top()))
				PRINT_ERR_RETURN(Msg::TypeMismatch);

			_stackValuesTypes.pop();
			_codeGen->SaveFromStack(varIt->second);
			break;

		case ARR_EL :
			//TO DO Maybe:)
			break;

		default :
			PRINT_ERR_RETURN(Msg::UnexpectedOperator);
			break;
		}

        return true;
    }

	bool Compiler :: PrintStatementProcess(PrintNode* node)
	{
		CHECK_SUCCESS(ExprProcess(node->expr));

		TypeNode argType = _stackValuesTypes.top();

		//Пока выводим только int, bool,char

		if (IsIntType(argType))
		{
			_codeGen->PrintInt();
		}
		else if (IsCharType(argType))
		{
			_codeGen->PrintChar();
		}
		else if (IsBoolType(argType))
		{
			_codeGen->PrintBool();
		}
		else
			PRINT_ERR_RETURN(Msg::PrintTypeMismatch);

		_stackValuesTypes.pop();
		return true;
	}

	bool Compiler :: FuncCallProcess(FuncCallNode* node)
    {
        return true;
    }

	bool Compiler :: IfStatementProcess(IfNode* node)
    {
        return true;
    }

	bool Compiler :: WhileStatementProcess(WhileDoNode* node)
    {
        return true;
    }

	bool Compiler :: ForStatementProcess(ForNode* node)
    {
        return true;
    }

	bool Compiler :: RepeatStatementProcess(RepeatNode* node)
    {
        return true;
    }

	bool Compiler :: CheckStatementProcess(CheckNode* node)
    {
        return true;
    }

	bool Compiler :: TypeMatch(const TypeNode& aT, const TypeNode& bT)
    {
		return (aT.type == bT.type) && (aT.dimen == bT.dimen);
	}

	bool Compiler :: IsCharOrIntType(const TypeNode& typeNode)
	{
		return IsCharType(typeNode) || IsIntType(typeNode);
	}

    bool Compiler :: FindVariable(const char* ident, Variable& var)
    {
        std::map<const char*, Variable, StrCmp>::iterator it = _scopeVars.find(ident);

        if (it == _scopeVars.end())
            return false;

        var = (*it).second;

        return true;
    }

	bool Compiler :: IsIntType(const TypeNode& type)
	{
		return (type.type == INT_TYPE) && (type.dimen == 0);
	}

	bool Compiler :: IsCharType(const TypeNode& type)
	{
		return (type.type == CHAR_TYPE) && (type.dimen == 0);
	}

	bool Compiler :: IsBoolType(const TypeNode& type)
	{
		return (type.type == BOOL_TYPE) && (type.dimen == 0);
	}

	int Compiler :: GetFreeTmpVar(const TypeNode& type)
	{
		for (std::set<int>::iterator it = _freeScopeTmpVars.begin(); it != _freeScopeTmpVars.end(); ++it)
		{
			if (TypeMatch(*_scopeTmpVars[*it]._type, type))
			{
				_freeScopeTmpVars.erase(it);
				return *it;
			}
		}

		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, Variable(_blockLocalsCount, new TypeNode(type), false)));
		_allScopeTmpVars.insert(_blockLocalsCount);

		return _blockLocalsCount++;
	}

	void Compiler :: RetrieveTmpVar(int num)
	{
		_freeScopeTmpVars.insert(num);
	}

} //L3Compiler namespace
