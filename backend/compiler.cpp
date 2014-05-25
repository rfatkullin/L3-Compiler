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
		ON_TRUE_ERR(!_program || !CheckMainFunc(), Msg::MainFuncIsNotDefined, _program->pos);

		return SubsDefProcess();
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
		ON_FALSE_ERR(mainSubNode->tag == FUNC, Msg::MainFuncMustReturnIntValue, mainSubNode->pos);

		//return int value
		ON_FALSE_ERR(IsIntType(*mainSubNode->type), Msg::MainFuncMustReturnIntValue, mainSubNode->pos);

		//take only one param
		ON_FALSE_ERR(mainSubNode->signature->params->size() == 1, Msg::MainFuncTakeOneStringArrParam, mainSubNode->pos);

		TypeNode paramType = *(mainSubNode->signature->params->begin()->second);

		//with type [[char]]
		ON_FALSE_ERR(paramType.type == CHAR_TYPE && paramType.dimen == 2, Msg::MainFuncTakeOneStringArrParam, mainSubNode->pos);

		return true;
	}	

	bool Compiler :: SubsDefProcess()
	{
		SubsDefNode* node = _program;

		_codeGen->Start();

		//Получаем все сигнатуры
		while (node)
		{
			CHECK_TRUE(SignatureProcess(node->def->type, node->def->signature));
			node = node->tail;
		};

		node = _program;
		//Генерация кода
		while (node)
		{
			CHECK_TRUE(SubDefNodeProcess(node->def));
			node = node->tail;
		};

		_codeGen->End();

		return true;
	}	

	bool Compiler :: SignatureProcess(TypeNode* type, SigNode* sig)
	{
		_codeGen->SubSignatureStart(type, sig->funcName);

		_currSubName = sig->funcName;

		SigNode::SubParams* params = sig->params;

		if (params->size() > 0)
		{
			SigNode::SubParams::iterator lastElIt = --params->end();
			for (SigNode::SubParams::iterator it = params->begin(); it != params->end(); ++it)
				_codeGen->SetSubParamDef(it->second, it != lastElIt);
		}

		_codeGen->SubSignatureEnd();

		_subs.insert(std::make_pair(sig->funcName, std::make_pair(type, sig->params)));

		return true;
	}

	bool Compiler :: AddSubParamsToScope(SigNode::SubParams* params, const std::pair<int, int>& pos)
	{
		for (SigNode::SubParams::iterator it = params->begin(); it != params->end(); ++it)
			CHECK_TRUE(AddScopeVar(it->first, _blockArgsCount++, it->second, true, pos));

		return true;
	}

	bool Compiler :: SubDefNodeProcess(SubDefNode* node)
	{
		EnterTheBlock();

		_currSubName = node->signature->funcName;

		AddSubParamsToScope(node->signature->params, node->pos);

		_codeGen->BlockStart(_currSubName);

		CHECK_TRUE(StatementsProcess(node->statements));

		if (node->tag == FUNC)
			ON_FALSE_ERR(_stackValuesTypes.size() > 0 && TypeMatch(_stackValuesTypes.top(), *node->type), Msg::SubSigReturnMismatch, node->pos);

		_codeGen->BlockEnd(node->signature->funcName, _scopeVars, node->tag == PROC);

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

				case LENGTH :
					CHECK_TRUE(LengthStatementProcess(stm->length));
					break;

				default :
					PRINT_ERR_RETURN(Msg::UnexpectedOperator, node->pos);
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

		CHECK_TRUE(AddScopeVar(node->ident, var, node->pos));

		switch (node->tag)
		{
			case IDENT :
				//Nothing to do - only add to locals map
				break;

			case EXPR :
				CHECK_TRUE(ExprAssignProcess(var, node->expr));
				break;

			default :
				PRINT_ERR_RETURN(Msg::IdentificatorExpected, node->pos);
				break;
		}

		return true;
	}

	bool Compiler :: ExprAssignProcess(Variable var, ExprNode* exprNode)
    {
		CHECK_TRUE(ExprProcess(exprNode));

		ON_FALSE_ERR(TypeMatch(_stackValuesTypes.top(), *var._type), Msg::TypeMismatch, exprNode->pos);

        _stackValuesTypes.pop();
        _codeGen->SaveFromStack(var);

        return true;
    }

	bool Compiler :: IsMulDivModOp(ExprNode* node)
	{
		return node->op == MULTIPLY || node->op == DIVIDE || node->op == MOD;
	}

	void Compiler :: ToLeftAssoc(ExprNode* node)
	{
		while (IsMulDivModOp(node->bin.right_expr))
		{
			ExprNode* expr = new ExprNode();
			expr->op = node->op;
			expr->bin.left_expr = node->bin.left_expr;
			expr->bin.right_expr = node->bin.right_expr->bin.left_expr;

			node->op = node->bin.right_expr->op;
			node->bin.left_expr = expr;
			node->bin.right_expr = node->bin.right_expr->bin.right_expr;
		}
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
		case EXPR :
			CHECK_TRUE(ExprProcess(node->un.expr));
			break;
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
			CHECK_TRUE(FindVariable(node->un.ident, tmpVar, node->pos));
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

			ON_FALSE_ERR(IsCharType(type1) || IsIntType(type1), Msg::UnaryMinusBadArgs, node->pos);

			_codeGen->NegOperator();
			_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			break;

		case FUNC_CALL :
			CHECK_TRUE(FuncCallProcess(node->un.func_call));
			break;

		case NEW_ARR :
			CHECK_TRUE(NewArrProcess(node->un.new_arr));
			break;

		case ARR_EL :
			CHECK_TRUE(GetArrElProcess(node->un.arr_el));
			break;

		case LENGTH :
			CHECK_TRUE(LengthStatementProcess(node->un.length));
			break;

		default :
			isUnary = false;
			break;
		}

		if (isUnary)
			return true;

		if (IsMulDivModOp(node))
			ToLeftAssoc(node);

		CHECK_TRUE(ExprProcess(node->bin.left_expr));
		CHECK_TRUE(ExprProcess(node->bin.right_expr));

		TypeNode type2 = _stackValuesTypes.top();
		_stackValuesTypes.pop();
		type1 = _stackValuesTypes.top();
		_stackValuesTypes.pop();

		switch (node->op)
		{
		case LOG_OR  :
		case LOG_AND :
		case LOG_CAP :

			ON_FALSE_ERR(IsBoolType(type1) && IsBoolType(type2), Msg::BoolOperatorsBadArgs, node->pos);

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

			_stackValuesTypes.push(TypeNode(BOOL_TYPE, 0));
			break;

		case EQ		:
		case NOT_EQ :

			CHECK_TRUE(EqOpArgCheck(type1, type2, node->pos));

			if (node->op == EQ)
				_codeGen->EqOperator();
			else
				_codeGen->NotEqOperator();

			_stackValuesTypes.push(TypeNode(BOOL_TYPE, 0));
			break;

		case LSS	:
		case GTR	:
		case LSS_EQ :
		case GTR_EQ :

			CHECK_TRUE(CompareOpArgCheck(type1, type2, node->pos));

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

			_stackValuesTypes.push(TypeNode(BOOL_TYPE, 0));
			break;

		case PLUS	:
			if (IsIntType(type1) && IsIntType(type2))
			{
				_codeGen->AddOperator();
				_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			}
			else if (IsIntType(type1)  && IsCharType(type2) ||
					 IsCharType(type1) && IsIntType(type2))
			{
				_codeGen->AddOperator();
				_stackValuesTypes.push(TypeNode(CHAR_TYPE, 0));
			}
			else
			{
				PRINT_ERR_RETURN(Msg::AddOperatorBadArgs, node->pos);
			}
			break;

		case MINUS	:
			if (IsIntType(type1)  && IsIntType(type2)  ||
				IsCharType(type1) && IsCharType(type2))
			{
				_codeGen->SubOperator();
				_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			}
			else if (IsCharType(type1) && IsIntType(type2))
			{
				_codeGen->SubOperator();
				_stackValuesTypes.push(TypeNode(CHAR_TYPE, 0));
			}
			else
			{
				PRINT_ERR_RETURN(Msg::SubOperatorBadArgs, node->pos);
			}
			break;

		case MULTIPLY :
		case DIVIDE	  :
		case MOD	  :
		case CAP	  :			

			ON_FALSE_ERR(IsIntType(type1) && IsIntType(type2), Msg::ArithmeticOperatorsBadArgs, node->pos);

			switch (node->op)
			{
				case MULTIPLY :
					_codeGen->MultOperator();
					break;
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

			_stackValuesTypes.push(TypeNode(INT_TYPE, 0));
			break;

		default :
			PRINT_ERR_RETURN(Msg::UnexpectedOperator, node->pos);
		}

		return true;
	}

	bool Compiler :: NewArrProcess(NewArrNode* node)
	{
		CHECK_TRUE(ExprProcess(node->expr));

		_codeGen->NewArr(node->type);

		_stackValuesTypes.pop();
		TypeNode type = *node->type;
		++type.dimen;
		_stackValuesTypes.push(type);

		return true;
	}

	bool Compiler :: GetArrElProcess(ArrElNode* node)
	{
		Variable var;
		CHECK_TRUE(FindVariable(node->ident, var, node->pos));

		_codeGen->LoadVariable(var);

		CHECK_TRUE(ArrElProcess(node->indexes));

		TypeNode type = *var._type;
		type.dimen -= node->indexes->size();

		_codeGen->LoadArrElem(type);

		_stackValuesTypes.push(type);

		return true;
	}

	bool Compiler :: ArrElProcess(std::list<ExprNode*>* indexes)
	{
		std::list<ExprNode*>::reverse_iterator lastEl = --indexes->rend();
		for (std::list<ExprNode*>::reverse_iterator it = indexes->rbegin(); it != indexes->rend(); ++it)
		{
			CHECK_TRUE(ExprProcess(*it));

			if (it != lastEl)
				_codeGen->LoadArrObj();

			_stackValuesTypes.pop();

		}

		return true;
	}

	bool Compiler :: AssignProcess(AssignNode* node)
	{
		TypeNode type;
		Variable var;

		switch (node->left->tag)
		{
		case IDENT :
			CHECK_TRUE(ExprProcess(node->expr));

			if (!strcmp(_currSubName, node->left->ident))
			{
				_codeGen->SetRet();
				return true;
			}

			CHECK_TRUE(FindVariable(node->left->ident, var, node->pos));
			ON_FALSE_ERR(TypeMatch(*var._type, _stackValuesTypes.top()), Msg::TypeMismatch, node->pos);

			_codeGen->SaveFromStack(var);
			break;

		case ARR_EL :
			CHECK_TRUE(FindVariable(node->left->arr_el->ident, var, node->pos));
			_codeGen->LoadVariable(var);
			CHECK_TRUE(ArrElProcess(node->left->arr_el->indexes))
			CHECK_TRUE(ExprProcess(node->expr));
			type = *var._type;
			type.dimen -= node->left->arr_el->indexes->size();
			ON_FALSE_ERR(TypeMatch(type, _stackValuesTypes.top()), Msg::TypeMismatch, node->pos);

			_codeGen->SaveArrElem(type);
			break;

		case FUNC_CALL_ARR_EL :
			CHECK_TRUE(FuncCallProcess(node->left->funcCallGetArrEl->funcCall))
			type = _stackValuesTypes.top();
			_stackValuesTypes.pop();
			CHECK_TRUE(ArrElProcess(node->left->funcCallGetArrEl->indexes))\
			CHECK_TRUE(ExprProcess(node->expr));
			type.dimen -= node->left->funcCallGetArrEl->indexes->size();
			ON_FALSE_ERR(TypeMatch(type, _stackValuesTypes.top()), Msg::TypeMismatch, node->pos);

			_codeGen->SaveArrElem(type);
			break;

		default :
			PRINT_ERR_RETURN(Msg::UnexpectedOperator, node->pos);
			break;
		}

		_stackValuesTypes.pop();

        return true;
    }

	bool Compiler :: PrintStatementProcess(PrintNode* node)
	{
		CHECK_TRUE(ExprProcess(node->expr));

		TypeNode argType = _stackValuesTypes.top();

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
		else if (IsStringType(argType))
		{
			_codeGen->PrintString();
		}
		else
			PRINT_ERR_RETURN(Msg::PrintOperatorBadArg, node->pos);

		_stackValuesTypes.pop();
		return true;
	}

	bool Compiler :: LengthStatementProcess(LengthNode* node)
	{
		CHECK_TRUE(ExprProcess(node->expr));

		ON_FALSE_ERR(_stackValuesTypes.top().dimen > 0, Msg::LengthOperatorBadArg, node->pos);

		_codeGen->LengthOperator();

		_stackValuesTypes.pop();
		_stackValuesTypes.push(TypeNode(INT_TYPE, 0));

		return true;
	}

	bool Compiler :: FuncCallProcess(FuncCallNode* node)
	{
		SubsMap::iterator funcRecIt = _subs.find(node->ident);

		ON_FALSE_ERR(funcRecIt != _subs.end(), Msg::UnknownSubCall, node->pos);

		SigNode::SubParams* funcParams = funcRecIt->second.second;
		std::list<ExprNode*>* factParams = node->params;

		ON_TRUE_ERR(factParams->size() > funcParams->size(), Msg::TooManyArgs, node->pos);
		ON_TRUE_ERR(factParams->size() < funcParams->size(), Msg::TooFewArgs, node->pos);

		std::list<ExprNode*>::iterator factParamIt = factParams->begin();
		for (SigNode::SubParams::iterator formParamIt = funcParams->begin(); formParamIt != funcParams->end(); ++formParamIt)
		{
			CHECK_TRUE(ExprProcess(*factParamIt));
			CHECK_TRUE(TypeMatch(_stackValuesTypes.top(), *formParamIt->second));

			++factParamIt;
			_stackValuesTypes.pop();
		}

		_codeGen->SetSubCall(funcRecIt->first, factParams->size());

		if (!IsVoidType(*funcRecIt->second.first))
			_stackValuesTypes.push(*funcRecIt->second.first);

        return true;
    }

	bool Compiler :: IfStatementProcess(IfNode* node)
	{
		int ifEndLabelNum;
		int next;
		std::stack<TypeNode> beginStackTypes = _stackValuesTypes;

		_codeGen->SaveStackDepth();

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
				_stackValuesTypes = beginStackTypes;
				_codeGen->RestoreStackDepth();
				_codeGen->SetLabel(next);

				CHECK_TRUE(ExprProcess(elseIfNode->expr));

				if (elseIfNode->else_if != NULL || node->suffix->statements != NULL )
					next = _codeGen->SetCondJumpToNewLabel(false);
				else
					_codeGen->SetCondJumpToLabel(ifEndLabelNum, false);

				_stackValuesTypes.pop();

				CHECK_TRUE(StatementsProcess(elseIfNode->statements));

				_codeGen->SetJumpTo(ifEndLabelNum);

				elseIfNode = elseIfNode->else_if;
			}

			//else
			if (node->suffix->statements != NULL)
			{
				_stackValuesTypes = beginStackTypes;
				_codeGen->RestoreStackDepth();
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
			ON_FALSE_ERR(varsDef->vars->tail == NULL, Msg::ForLoopDefOnlyOneVar, node->pos);
			ON_FALSE_ERR(IsIntType(*varsDef->type) || IsCharType(*varsDef->type), Msg::ForLoopVariableMustBeIntOrChar, node->pos);
			CHECK_TRUE(VarDefsProcess(varsDef));
			forLoopVarName = varsDef->vars->var->ident;
		}

		Variable counterVar;
		CHECK_TRUE(FindVariable(forLoopVarName, counterVar, node->_fromParam->pos));

		Variable toExprVar = GetFreeTmpVar(*counterVar._type);
		Variable stepExprVar = GetFreeTmpVar(TypeNode(INT_TYPE, 0));

		CHECK_TRUE(CompareOpArgCheck(*counterVar._type, *toExprVar._type, node->_fromParam->pos));
		CHECK_TRUE(AddOpArgCheck(*counterVar._type, *stepExprVar._type, node->_fromParam->pos));

		CHECK_TRUE(ExprProcess(node->_toParam->to));
		_codeGen->SaveFromStack(toExprVar);
		_stackValuesTypes.pop();

		CHECK_TRUE(ExprProcess(node->_toParam->step));
		_codeGen->SaveFromStack(stepExprVar);
		_stackValuesTypes.pop();

		int loopBeginCondLabel =_codeGen->SetNewLabel();

		_codeGen->LoadVariable(counterVar);
		_codeGen->LoadVariable(toExprVar);
		_codeGen->GtrOperator();
		int LoopEndLabelNum =_codeGen->SetCondJumpToNewLabel(true);

		CHECK_TRUE(StatementsProcess(node->_statements));

		_codeGen->LoadVariable(counterVar);
		_codeGen->LoadVariable(stepExprVar);
		_codeGen->AddOperator();
		_codeGen->SaveFromStack(counterVar);
		_codeGen->SetJumpTo(loopBeginCondLabel);
		_codeGen->SetLabel(LoopEndLabelNum);

        return true;
    }

	bool Compiler :: AddScopeVar(const char* ident, int id, TypeNode* type, bool isArg, const std::pair<int, int>& pos)
	{
		CHECK_TRUE(AddScopeVar(ident, Variable(id, type, isArg), pos));

		return true;
	}

	bool Compiler :: AddScopeVar(const char* ident, const Variable& var, const std::pair<int, int>& pos)
	{
		ON_TRUE_ERR(_scopeVars.find(ident) != _scopeVars.end(), Msg::DeclarationConflict, pos);

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

	bool Compiler :: FindVariable(const char* ident, Variable& var, const std::pair<int, int>& pos)
    {
		std::map<const char*, Variable, StrCmp>::iterator locIt = _scopeVars.find(ident);

		ON_TRUE_ERR(locIt == _scopeVars.end(), Msg::VariableNotDiclared, pos);

		var = (*locIt).second;

        return true;
    }

	bool Compiler :: AddOpArgCheck(const TypeNode& type1, const TypeNode& type2, const std::pair<int, int>& pos)
	{
		ON_FALSE_ERR(IsIntType(type1) && IsIntType(type2)  ||
					 IsIntType(type1) && IsCharType(type2) ||
					 IsCharType(type1) && IsIntType(type2), Msg::AddOperatorBadArgs, pos);

		return true;
	}


	bool Compiler :: EqOpArgCheck(const TypeNode& type1, const TypeNode& type2, const std::pair<int, int>& pos)
	{
		ON_FALSE_ERR(TypeMatch(type1, type2)  ||
					 (IsCharOrIntType(type1) && IsCharOrIntType(type2)), Msg::EqOperatorBadArgs, pos);

		return true;
	}

	bool Compiler :: CompareOpArgCheck(const TypeNode& type1, const TypeNode& type2, const std::pair<int, int>& pos)
	{
		ON_FALSE_ERR(IsCharOrIntType(type1) && IsCharOrIntType(type2), Msg::CompareOperatorBadArgs, pos);

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

	bool Compiler :: IsStringType(const TypeNode& type)
	{
		return (type.type == CHAR_TYPE) && (type.dimen == 1);
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
		const char* varName = GetTmpVarName(_blockLocalsCount);
		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, var));
		AddScopeVar(varName, var, std::make_pair(-1, -1));
		_allScopeTmpVars.insert(_blockLocalsCount++);

		return var;
	}

	const char* Compiler :: GetTmpVarName(int id) const
	{
		std::string nameString = "^_" + CodeGenerator::IntToStr(_blockLocalsCount);
		int nameLength = nameString.length();
		char* name = new char[nameLength + 1];
		strncpy(name, nameString.c_str(), nameLength + 1);

		return name;
	}

	void Compiler :: RetrieveTmpVar(int num)
	{
		_freeScopeTmpVars.insert(num);
	}

} //L3Compiler namespace
