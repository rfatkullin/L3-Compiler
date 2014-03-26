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

		if (!CheckMainFunc())
		{
			printf("Can't find main func\n");
			return false;
		}

		return true;
	}

	bool Compiler :: CheckMainFunc()
	{
		SubsDefNode* node = _program;
		SubDefNode* mainSubNode = NULL;

		while (node)
		{
			if (!strcmp("main", node->def->signature->funcName))
			{
				mainSubNode = node->def;
				break;
			}

			node = node->tail;
		}

		if (mainSubNode == NULL)
			return false;

		//Main must be function -> int main([[char]])
		ON_FALSE_ERR(mainSubNode->tag == FUNC, Msg::MainFuncMustReturnIntValue);

		//return int value
		ON_FALSE_ERR(IsIntType(*mainSubNode->type), Msg::MainFuncMustReturnIntValue);

		//take only one param
		ON_FALSE_ERR(mainSubNode->signature->params->size() == 1, Msg::MainFuncTakeOneStringArrParam);

		TypeNode paramType = *(mainSubNode->signature->params->begin()->second);

		//with type [[char]]
		ON_FALSE_ERR(paramType.type == CHAR_TYPE && paramType.dimen == 2, Msg::MainFuncTakeOneStringArrParam);

		return true;
	}	

	bool Compiler :: SubsDefProcess()
	{
		SubsDefNode* node = _program;

		_codeGen->Start();

		while (node)
		{
			CHECK_TRUE(SubDefNodeProcess(node->def));
			node = node->tail;
		};

		_codeGen->End();

		return true;
	}

	bool Compiler :: SubDefNodeProcess(SubDefNode* node)
	{
		EnterTheBlock();

		_codeGen->SubSignatureStart(node->type);

		CHECK_TRUE(SignatureProcess(node->type, node->signature));

		_codeGen->BlockStart();

		CHECK_TRUE(StatementsProcess(node->statements));

		if (node->tag == FUNC)
			ON_FALSE_ERR(_stackValuesTypes.size() > 0 && TypeMatch(_stackValuesTypes.top(), *node->type), Msg::SubSigReturnMismatch);

		_codeGen->BlockEnd(node->signature->funcName, _scopeVars, _blockLocalsCount, node->tag == PROC);

		return true;
	}

	bool Compiler :: SignatureProcess(TypeNode* type, SigNode* sig)
	{		
		_codeGen->SubSignatureStart(type);
		_codeGen->SetSubName(sig->funcName);

		_currSubName = sig->funcName;

		SigNode::SubParams* params = sig->params;
		SigNode::SubParams::iterator lastElIt = --params->end();
		for (SigNode::SubParams::iterator it = params->begin(); it != params->end(); ++it)
		{
			CHECK_TRUE(AddScopeVar(it->first, _blockArgsCount++, it->second, true));

			_codeGen->SetSubParamDef(it->second, it != lastElIt);
		}

		_codeGen->SubSignatureEnd();

		_subs.insert(std::make_pair(sig->funcName, std::make_pair(type, sig->params)));

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

	bool Compiler :: StatementsProcess(StatementsNode* node)
	{
        while (node)
		{
            StatementNode* stm = node->statement;

			//Clear stack after func/subs call without assign operator
			while (!_stackValuesTypes.empty())
			{
				_stackValuesTypes.pop();
				_codeGen->PopFromStack();
			}

            switch (stm->tag)
			{
				case VARS_DEF :
					CHECK_TRUE(VarDefsProcess(stm->vars_def));
					break;

				case ASSIGN :
					CHECK_TRUE(AssignProcess(stm->assign));
					break;

				case FUNC_CALL :
					CHECK_TRUE(FuncCallProcess(stm->func_call));
					break;

				case IF :
					CHECK_TRUE(IfStatementProcess(stm->if_statement));
					break;

				case WHILE :
					CHECK_TRUE(WhileStatementProcess(stm->while_do));
					break;

				case FOR :
					CHECK_TRUE(ForStatementProcess(stm->for_statement));
					break;

				case REPEAT :
					CHECK_TRUE(RepeatStatementProcess(stm->repeat));
					break;

				case CHECK :
					CHECK_TRUE(CheckStatementProcess(stm->check));
					break;

				case PRINT :
					CHECK_TRUE(PrintStatementProcess(stm->print));
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
			CHECK_TRUE(VarDefProcess(vars->var, node->type));
			vars = vars->tail;
		}

		return true;
	}

	bool Compiler :: VarDefProcess(VarNode* node, TypeNode* type)
	{
		Variable var(_blockLocalsCount++, type, false);

		CHECK_TRUE(AddScopeVar(node->ident, var));

		switch (node->tag)
		{
			case IDENT :
				//Nothing to do - only add to locals map
				break;

			case EXPR :
				CHECK_TRUE(ExprAssignProcess(var, node->expr));
				break;

			case NEW_ARR :
				CHECK_TRUE(NewArrAssignProcess(var, node->new_arr));
				break;

			default :
				PRINT_ERR_RETURN(Msg::IdentificatorExpected);
				break;
		}

		return true;
	}

	bool Compiler :: ExprAssignProcess(Variable var, ExprNode* exprNode)
    {
		CHECK_TRUE(ExprProcess(exprNode));

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
		Variable tmpVar;
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
			ON_FALSE_ERR(FindVariable(node->un.ident, tmpVar), Msg::VariableNotDiclared);
			_stackValuesTypes.push(*tmpVar._type);
			_codeGen->LoadVariable(tmpVar);
			break;

		case EXCL :
			CHECK_TRUE(ExprProcess(node->un.expr));
			CHECK_TRUE(IsBoolType(_stackValuesTypes.top()));
			_codeGen->NotOperator();
			break;

		case UMINUS :
			CHECK_TRUE(ExprProcess(node->un.expr));

			type1 = _stackValuesTypes.top();
			_stackValuesTypes.pop();

			CHECK_TRUE(IsBoolType(type1) || IsIntType(type1));

			_codeGen->NegOperator();
			_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			break;

		case FUNC_CALL :
			CHECK_TRUE(FuncCallProcess(node->un.func_call));
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

		CHECK_TRUE(ExprProcess(node->bin.left_expr));
		CHECK_TRUE(ExprProcess(node->bin.right_expr));

		TypeNode type2 = _stackValuesTypes.top();
		_stackValuesTypes.pop();
		type1 = _stackValuesTypes.top();		

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
					tmpVar = GetFreeTmpVar(TypeNode(INT_TYPE, 0));
					_codeGen->PowOperator(tmpVar._id);
					RetrieveTmpVar(tmpVar._id);
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

		CHECK_TRUE(ExprProcess(node->expr));

		switch (node->left->tag)
		{
		case IDENT :
			varIt = _scopeVars.find(node->left->ident);

			if (varIt == _scopeVars.end())
			{
				if (!strcmp(_currSubName, node->left->ident))
				{
					_codeGen->SetRet();
					return true;
				}

				PRINT_ERR_RETURN(Msg::VariableNotDiclared);
			}

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
		CHECK_TRUE(ExprProcess(node->expr));

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
		SubsMap::iterator funcRecIt = _subs.find(node->ident);

		ON_FALSE_ERR(funcRecIt != _subs.end(), Msg::UnknownSubCall);

		SigNode::SubParams* funcParams = funcRecIt->second.second;
		std::list<ExprNode*>* factParams = node->params;

		ON_TRUE_ERR(factParams->size() > funcParams->size(), Msg::TooManyArgs);
		ON_TRUE_ERR(factParams->size() < funcParams->size(), Msg::TooFewArgs);

		std::list<ExprNode*>::iterator factParamIt = factParams->begin();
		for (SigNode::SubParams::iterator formParamIt = funcParams->begin(); formParamIt != funcParams->end(); ++formParamIt)
		{
			CHECK_TRUE(ExprProcess(*factParamIt));
			CHECK_TRUE(TypeMatch(_stackValuesTypes.top(), *formParamIt->second));

			++factParamIt;
			_stackValuesTypes.pop();
		}

		_codeGen->SetSubCall(funcRecIt->first);

		if (!IsVoidType(*funcRecIt->second.first))
			_stackValuesTypes.push(*funcRecIt->second.first);

        return true;
    }

	bool Compiler :: IfStatementProcess(IfNode* node)
	{
		int ifEndLabelNum;
		int next;

		//if
		CHECK_TRUE(ExprProcess(node->expr));
		next = _codeGen->SetCondJumpToNewLabel(false);
		_stackValuesTypes.pop();
		CHECK_TRUE(StatementsProcess(node->statements));

		if (node->suffix == NULL)
			_codeGen->SetLabel(next);
		else //elseif
		{
			ifEndLabelNum = _codeGen->SetJumpToNewLabel();

			ElseIfNode* elseIfNode = node->suffix->else_if;
			while (elseIfNode)
			{
				_codeGen->SetLabel(next);

				CHECK_TRUE(ExprProcess(elseIfNode->expr));

				if (elseIfNode->else_if != NULL || node->suffix->statements != NULL )
					next = _codeGen->SetCondJumpToNewLabel(false);
				else
					_codeGen->SetJumpTo(ifEndLabelNum);

				_stackValuesTypes.pop();

				CHECK_TRUE(StatementsProcess(elseIfNode->statements));

				_codeGen->SetJumpTo(ifEndLabelNum);

				elseIfNode = elseIfNode->else_if;
			}

			//else
			if (node->suffix->statements != NULL)
			{
				_codeGen->SetLabel(next);
				CHECK_TRUE(StatementsProcess(node->suffix->statements));
			}

			_codeGen->SetLabel(ifEndLabelNum);
		}

		return true;
    }

	bool Compiler :: WhileStatementProcess(WhileDoNode* node)
    {
		int startLabelNum = _codeGen->SetNewLabel();

		CHECK_TRUE(ExprProcess(node->expr));

		int afterLoopLabel = _codeGen->SetCondJumpToNewLabel(false);

		CHECK_TRUE(StatementsProcess(node->statements));

		_codeGen->SetJumpTo(startLabelNum);
		_codeGen->SetLabel(afterLoopLabel);

        return true;
    }

	bool Compiler :: ForStatementProcess(ForNode* node)
	{
		const char* forLoopVarName = NULL;

		if (node->_fromParam->_type == ASSIGN)
		{
			CHECK_TRUE(AssignProcess(node->_fromParam->_assign));
			forLoopVarName = node->_fromParam->_assign->left->ident;
		}
		else
		{
			VarsDefNode* varsDef = node->_fromParam->_varsDef;
			ON_FALSE_ERR(varsDef->vars->tail == NULL, Msg::ForLoopDefOnlyOneVar);
			ON_FALSE_ERR(IsIntType(*varsDef->type) || IsCharType(*varsDef->type), Msg::ForLoopVariableMustBeIntOrChar);
			CHECK_TRUE(VarDefsProcess(varsDef));
			forLoopVarName = varsDef->vars->var->ident;
		}

		Variable counterVar;
		CHECK_TRUE(FindVariable(forLoopVarName, counterVar));

		Variable toExprVar = GetFreeTmpVar(*counterVar._type);
		Variable stepExprVar = GetFreeTmpVar(TypeNode(INT_TYPE, 0));

		CHECK_TRUE(ExprProcess(node->_toParam->to));
		_codeGen->SaveFromStack(toExprVar);

		CHECK_TRUE(ExprProcess(node->_toParam->step));
		_codeGen->SaveFromStack(stepExprVar);

		int loopBeginCondLabel =_codeGen->SetNewLabel();

		_codeGen->LoadVariable(counterVar);
		_codeGen->LoadVariable(toExprVar);
		_codeGen->GtrOperator();
		int LoopEndLabelNum =_codeGen->SetCondJumpToNewLabel(true);

		CHECK_TRUE(StatementsProcess(node->_statements));

		_codeGen->LoadVariable(counterVar);
		_codeGen->LoadVariable(stepExprVar);
		_codeGen->Add();
		_codeGen->SaveFromStack(counterVar);
		_codeGen->SetJumpTo(loopBeginCondLabel);
		_codeGen->SetLabel(LoopEndLabelNum);

        return true;
    }

	bool Compiler :: AddScopeVar(const char* ident, int id, TypeNode* type, bool isArg)
	{
		ON_TRUE_ERR(_scopeVars.find(ident) != _scopeVars.end(), Msg::DeclarationConflict);

		_scopeVars.insert(std::pair<const char*, Variable>(ident, Variable(id, type, isArg)));

		return true;
	}

	bool Compiler :: AddScopeVar(const char* ident, const Variable& var)
	{
		ON_TRUE_ERR(_scopeVars.find(ident) != _scopeVars.end(), Msg::DeclarationConflict);

		_scopeVars.insert(std::pair<const char*, Variable>(ident, var));

		return true;
	}

	bool Compiler :: RepeatStatementProcess(RepeatNode* node)
    {
		int loopBeginLabel =_codeGen->SetNewLabel();

		CHECK_TRUE(StatementsProcess(node->statements));
		CHECK_TRUE(ExprProcess(node->expr));
		_codeGen->SetCondJumpToLabel(loopBeginLabel, false);

        return true;
    }

	bool Compiler :: CheckStatementProcess(CheckNode* node)
    {
		CHECK_TRUE(ExprProcess(node->expr));

		_codeGen->ExitOn(false);

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

	bool Compiler :: IsVoidType(const TypeNode& type)
	{
		return (type.type == VOID_TYPE) && (type.dimen == 0);
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

	Variable Compiler :: GetFreeTmpVar(const TypeNode& type)
	{
		for (std::set<int>::iterator it = _freeScopeTmpVars.begin(); it != _freeScopeTmpVars.end(); ++it)
		{
			if (TypeMatch(*_scopeTmpVars[*it]._type, type))
			{
				_freeScopeTmpVars.erase(it);
				return _scopeTmpVars[*it];
			}
		}

		Variable var(_blockLocalsCount, new TypeNode(type), false);
		std::string varName = "^^^" + CodeGenerator::IntToStr(_blockLocalsCount) + "^^^";
		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, var));
		_scopeVars.insert(std::pair<const char*, Variable>(varName.c_str(), var));
		_allScopeTmpVars.insert(_blockLocalsCount++);

		return var;
	}

	void Compiler :: RetrieveTmpVar(int num)
	{
		_freeScopeTmpVars.insert(num);
	}

} //L3Compiler namespace
