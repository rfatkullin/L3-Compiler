#ifndef _NODE_H_
#define _NODE_H_

#include <list>
#include <string>
#include <stdio.h>

class SubDefNode;
class FuncDefNode;
class ProcDefNode;
class SigNode;
class TypeNode;
class StatementsNode;
class ParamSeqNode;
class ParamsDefNode;
class VarsDefNode;
class AssignNode;
class FuncCallNode;
class IfNode;
class WhileDoNode;
class ForNode;
class RepeatNode;
class CheckNode;
class PrintNode;
class IdentsNode;
class StatementNode;
class WhileDoNode;
class ExprNode;
class IfSuffixNode;
class ElseIfNode;
class ForFromParamNode;
class ForToParamNode;
class VarsNode;
class VarNode;
class NewArrNode;
class LeftValueNode;
class ArrElNode;
class FuncCallGetArrElNode;
class LengthNode;

class Node
{
public :
	std::pair<int, int> pos;

	void SetLines(int startLine, int endLine)
	{
		pos = std::make_pair(startLine, endLine);
	}
};

class SubsDefNode : public Node
{
public :
	SubsDefNode( SubDefNode* new_def, SubsDefNode* new_tail )
		: 	def( new_def ),
			tail( new_tail )
			{ }

	SubDefNode* def;
	SubsDefNode* tail;
};

class SubDefNode : public Node
{
public :

	SubDefNode(int newTag, SigNode* newSig, TypeNode* newType, StatementsNode* newStatements)
		: tag(newTag),
		  signature(newSig),
		  type(newType),
		  statements(newStatements)
	{}

	int				tag;
	SigNode* 		signature;
	TypeNode* 		type;
	StatementsNode*	statements;
};

class ParamSeqNode : public Node
{
public :
	ParamSeqNode( IdentsNode* new_idents, TypeNode* new_type )
		: 	idents( new_idents ),
			type( new_type )
			{}

	IdentsNode* 	idents;
	TypeNode* 		type;
};

class ParamsDefNode : public Node
{
public :
	ParamsDefNode( ParamSeqNode* new_params_sec, ParamsDefNode* new_tail )
		:	params_seq( new_params_sec ),
			tail( new_tail )
			{}

	ParamSeqNode* 	params_seq;
	ParamsDefNode* 	tail;
};

class IdentsNode : public Node
{
public :
	IdentsNode( char* new_ident, IdentsNode* new_tail )
		:	ident( new_ident ),
			tail( new_tail )
			{}

	char* 		ident;
	IdentsNode* tail;
};

class SigNode : public Node
{
public :
	typedef std::list<std::pair<const char*,TypeNode*> > SubParams;

	SigNode( char* newFuncName, ParamsDefNode* newParamsDef )
		: 	funcName( newFuncName )
			{
				params = new std::list<std::pair<const char*,TypeNode*> >();

				ParamsDefNode* groups = newParamsDef;
				ParamsDefNode* groupsTail = NULL;
				while (groups)
				{
					ParamSeqNode* paramsSeq = groups->params_seq;
					IdentsNode* idents = paramsSeq->idents;
					IdentsNode* identsTail = NULL;

					while (idents)
					{
						params->push_back(std::make_pair(idents->ident, paramsSeq->type));
						identsTail = idents->tail;
						delete idents;
						idents = identsTail;
					}
					groupsTail = groups->tail;
					delete groups;
					groups = groupsTail;
				}
			}

	char* funcName;
	SubParams* params;
};

class StatementsNode : public Node
{
public :
	StatementsNode( StatementNode* new_statement, StatementsNode* new_tail )
		: 	statement( new_statement ),
			tail( new_tail )
			{}

	StatementNode*  statement;
	StatementsNode* tail;
};

class StatementNode : public Node
{
public :
	StatementNode( int new_tag )
		: 	tag( new_tag )
		{}

	int tag;

	union
	{
		VarsDefNode* 	vars_def;
		AssignNode* 	assign;
		FuncCallNode*	func_call;
		IfNode*			if_statement;
		WhileDoNode*	while_do;
		ForNode*		for_statement;
		RepeatNode*		repeat;
		CheckNode*		check;
		PrintNode*		print;
		LengthNode*		length;
	};
};

class CheckNode : public Node
{
public :
	CheckNode( ExprNode* new_expr )
		:	expr( new_expr )
		{}

	ExprNode* expr;
};

class PrintNode : public Node
{
public :
	PrintNode(ExprNode* toPrintExpr)
		: expr(toPrintExpr)
		{}

	ExprNode* expr;
};

class LengthNode : public Node
{
public :
	LengthNode(ExprNode* newExpr)
		: expr(newExpr)
		{}

	ExprNode* expr;
};

class WhileDoNode : public Node
{
public :
	WhileDoNode( ExprNode* new_expr, StatementsNode* new_statements )
		: 	expr( new_expr ),
			statements( new_statements )
		{}

	ExprNode* 			expr;
	StatementsNode* 	statements;
};

class IfNode : public Node
{
public :
	IfNode( ExprNode* new_expr, StatementsNode* new_statements, IfSuffixNode* new_suffix )
		:	expr( new_expr ),
			statements( new_statements ),
			suffix( new_suffix )
		{}

	ExprNode* 			expr;
	StatementsNode* 	statements;
	IfSuffixNode* 		suffix;
};

class IfSuffixNode : public Node
{
public :
	IfSuffixNode( ElseIfNode* new_else_if, StatementsNode* new_statements )
		: 	else_if( new_else_if ),
			statements( new_statements )
		{}

	ElseIfNode* 	else_if;
	StatementsNode* statements;
};

class ElseIfNode : public Node
{
public :
	ElseIfNode( ExprNode* new_expr, StatementsNode* new_statements, ElseIfNode* new_else_if )
		:	expr( new_expr ),
			statements( new_statements ),
			else_if( new_else_if )
		{}

	ExprNode* 		expr;
	StatementsNode* 	statements;
	ElseIfNode* 		else_if;
};

class ForNode : public Node
{
public :
	ForNode(ForFromParamNode* fromParam, ForToParamNode* toParam, StatementsNode* statements)
		:	_fromParam(fromParam),
			_toParam(toParam),
			_statements(statements)
	{}

	ForFromParamNode* 	_fromParam;
	ForToParamNode*		_toParam;
	StatementsNode*		_statements;
};

class ForFromParamNode : public Node
{
public :
	ForFromParamNode(int type)
		:	_type(type)
	{}

	int _type;

	union
	{
		AssignNode* _assign;
		VarsDefNode* _varsDef;

	};
};

class ForToParamNode : public Node
{
public :
	ForToParamNode( ExprNode* new_to, ExprNode* new_step )
		: 	to( new_to ),
			step( new_step )
		{}

	ExprNode* to;
	ExprNode* step;
};

class VarsDefNode : public Node
{
public :
	VarsDefNode( VarsNode* new_vars, TypeNode* new_type )
		: 	vars( new_vars ),
			type( new_type )
		{}

	VarsNode* vars;
	TypeNode* type;
};

class VarsNode : public Node
{
public :
	VarsNode( VarNode* new_var, VarsNode* new_tail )
		:	var( new_var ),
			tail( new_tail )
		{}

	VarNode* 	var;
	VarsNode* 	tail;
};

class VarNode : public Node
{
public :
	VarNode( int new_tag, char* new_ident )
		: 	tag( new_tag ),
			ident( new_ident )
		{}

	int 	tag;
	char*	ident;

	union
	{
		ExprNode*	expr;
		NewArrNode*	new_arr;
	};
};

class NewArrNode : public Node
{
public :
	NewArrNode(TypeNode* newType, ExprNode* newExpr)
		:	type(newType),
			expr(newExpr)
		{}

	TypeNode* type;
	ExprNode* expr;
};

class AssignNode : public Node
{
public :
	AssignNode(LeftValueNode* new_left, ExprNode* new_expr)
		: 	left(new_left),
			expr(new_expr)
		{}

	LeftValueNode* 	left;
	ExprNode* 		expr;
};

class LeftValueNode : public Node
{
public :
	LeftValueNode(int new_tag)
		: tag(new_tag)
		{}

	int tag;	

	union
	{
		char* 		ident;
		ArrElNode*	arr_el;
		FuncCallGetArrElNode* funcCallGetArrEl;
	};
};

class FuncCallGetArrElNode : public Node
{
public :
	FuncCallGetArrElNode(FuncCallNode* newFuncCall, std::list<ExprNode*>* newIndexes)
		: funcCall(newFuncCall),
		  indexes(newIndexes)
		{}

	FuncCallNode* funcCall;
	std::list<ExprNode*>* indexes;
};

class ArrElNode : public Node
{
public :
	ArrElNode(char* newIdent, std::list<ExprNode*>* newIndexes)
		: 	ident(newIdent),
			indexes(newIndexes)
		{}

	char*					ident;
	std::list<ExprNode*>*	indexes;
};

class RepeatNode : public Node
{
public :
	RepeatNode( StatementsNode* new_statements, ExprNode* new_expr )
		:	statements( new_statements ),
			expr( new_expr )
		{}

	StatementsNode* statements;
	ExprNode* 		expr;
};

class TypeNode : public Node
{
public :
	TypeNode()
		:	type(-1),
			dimen(-1)
	{}

	TypeNode( int new_type, int new_dimen )
		: 	type( new_type ),
			dimen( new_dimen )
	{}    

	int type;
	int dimen;
};

class ExprNode : public Node
{
public :
	int op;

	union
	{
		struct
		{
			ExprNode* 	left_expr;
			ExprNode* 	right_expr;
		} bin;

		union
		{
			char*			str;
			char* 			ident;
			char 			ch;
			int 			num;
			bool			log;
			ExprNode* 		expr;
			ArrElNode* 		arr_el;
			NewArrNode*		new_arr;
			FuncCallNode* 	func_call;
			LengthNode*		length;
		} un;
	};
};

class FuncCallNode : public Node
{
public :
	FuncCallNode(char* newIdent, std::list<ExprNode*>* newParams)
		: 	ident(newIdent),
			params(newParams)
		{}

	char* ident;
	std::list<ExprNode*>* params;
};

#endif
