#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <stdio.h>

class SubDefNode;
class FuncDefNode;
class ProcDefNode;
class SigNode;
class TypeNode;
class StatementsNode;
class ParamsDefNode;
class ParamSecNode;
class ParamsDefNode;
class VarsDefNode;
class AssignNode;
class FuncCallNode;
class IfNode;
class WhileDoNode;
class ForNode;
class RepeatNode;
class CheckNode;
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
class NewArrBasic;
class LeftValueNode;
class ArrElNode;

class SubsDefNode
{
public :
	SubsDefNode( SubDefNode* new_def, SubsDefNode* new_tail )
		: 	def( new_def ),
			tail( new_tail )
			{ }

	SubDefNode* def;
	SubsDefNode* tail;
};

class SubDefNode
{
public :

	SubDefNode( int new_tag )
		: tag( new_tag )
	{}

	int tag;

	union
	{
		FuncDefNode* func;
		ProcDefNode* proc;
	};
};

class FuncDefNode
{
public :
	FuncDefNode( SigNode* new_signature, TypeNode* new_type, StatementsNode* new_statements )
		:	signature( new_signature ),
			type( new_type ),
			statements( new_statements )
			{}

	SigNode* 		signature;
	TypeNode* 		type;
	StatementsNode*	statements;
};

class ProcDefNode
{
public :
	ProcDefNode( SigNode* new_signature, StatementsNode* new_statements )
		: 	signature( new_signature ),
			statements( new_statements )
			{}

	SigNode* 		signature;
	StatementsNode*	statements;
};

class SigNode
{
public :
	SigNode( char* new_ident, ParamsDefNode* new_params_def )
		: 	ident( new_ident ),
			params_def( new_params_def )
			{}

	char* 			ident;
	ParamsDefNode* 	params_def;
};

class ParamsDefNode
{
public :
	ParamsDefNode( ParamSecNode* new_params_sec, ParamsDefNode* new_tail )
		:	params_sec( new_params_sec ),
			tail( new_tail )
			{}

	ParamSecNode* 	params_sec;
	ParamsDefNode* 	tail;
};

class ParamSecNode
{
public :
	ParamSecNode( IdentsNode* new_idents, TypeNode* new_type )
		: 	idents( new_idents ),
			type( new_type )
			{}

	IdentsNode* 	idents;
	TypeNode* 		type;
};

class IdentsNode
{
public :
	IdentsNode( char* new_ident, IdentsNode* new_tail )
		:	ident( new_ident ),
			tail( new_tail )
			{}

	char* 		ident;
	IdentsNode* tail;
};

class StatementsNode
{
public :
	StatementsNode( StatementNode* new_statement, StatementsNode* new_tail )
		: 	statement( new_statement ),
			tail( new_tail )
			{}

	StatementNode*  statement;
	StatementsNode* tail;
};

class StatementNode
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
	};
};

class CheckNode
{
public :
	CheckNode( ExprNode* new_expr )
		:	expr( new_expr )
		{}

	ExprNode* expr;
};

class WhileDoNode
{
public :
	WhileDoNode( ExprNode* new_expr, StatementsNode* new_statements )
		: 	expr( new_expr ),
			statements( new_statements )
		{}

	ExprNode* 			expr;
	StatementsNode* 	statements;
};

class IfNode
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

class IfSuffixNode
{
public :
	IfSuffixNode( ElseIfNode* new_else_if, StatementsNode* new_statements )
		: 	else_if( new_else_if ),
			statements( new_statements )
		{}

	ElseIfNode* 	else_if;
	StatementsNode* statements;
};

class ElseIfNode
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

class ForNode
{
public :
	ForNode( ForFromParamNode* new_from_param, ForToParamNode* new_to_param, StatementsNode* new_statements )
		:	from_param( new_from_param ),
			to_param( new_to_param ),
			statements( new_statements )
		{}

	ForFromParamNode* 	from_param;
	ForToParamNode* 	to_param;
	StatementsNode*  	statements;
};

class ForFromParamNode
{
public :
	ForFromParamNode( char* new_ident, ExprNode* new_expr, TypeNode* new_type )
		: 	ident( new_ident ),
			expr( new_expr ),
			type( new_type )
		{}

	char* 		ident;
	ExprNode* 	expr;
	TypeNode* 	type;
};

class ForToParamNode
{
public :
	ForToParamNode( ExprNode* new_to, ExprNode* new_step )
		: 	to( new_to ),
			step( new_step )
		{}

	ExprNode* to;
	ExprNode* step;
};

class VarsDefNode
{
public :
	VarsDefNode( VarsNode* new_vars, TypeNode* new_type )
		: 	vars( new_vars ),
			type( new_type )
		{}

	VarsNode* vars;
	TypeNode* type;
};

class VarsNode
{
public :
	VarsNode( VarNode* new_var, VarsNode* new_tail )
		:	var( new_var ),
			tail( new_tail )
		{}

	VarNode* 	var;
	VarsNode* 	tail;
};

class VarNode
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

class NewArrNode
{
public :
	NewArrNode( int new_dimen, NewArrBasic* new_arr_basic )
		:	dimen( new_dimen ),
			arr_basic( new_arr_basic )
		{}

	int 			dimen;
	NewArrBasic* 	arr_basic;
};

class NewArrBasic
{
public :
	NewArrBasic( TypeNode* new_type, ExprNode* new_expr )
		: 	type( new_type ),
			expr( new_expr )
		{}

	TypeNode* type;
	ExprNode* expr;
};

class AssignNode
{
public :
	AssignNode( LeftValueNode* 	new_left, ExprNode* new_expr )
		: 	left( new_left ),
			expr( new_expr )
		{}

	LeftValueNode* 	left;
	ExprNode* 		expr;
};

class LeftValueNode
{
public :
	LeftValueNode( int new_tag )
		:	tag( new_tag )
		{}

	int tag;

	union
	{
		char* 		ident;
		ArrElNode*	arr_el;
	};
};

class ArrElNode
{
public :
	ArrElNode( char* new_ident, ExprNode* new_index )
		: 	ident( new_ident ),
			index( new_index )
		{}

	char* 		ident;
	ExprNode*	index;
};

class RepeatNode
{
public :
	RepeatNode( StatementsNode* new_statements, ExprNode* new_expr )
		:	statements( new_statements ),
			expr( new_expr )
		{}

	StatementsNode* statements;
	ExprNode* 		expr;
};

class TypeNode
{
public :
	TypeNode( int new_type, int new_dimen )
		: 	type( new_type ),
			dimen( new_dimen )
	{}    

	int type;
	int dimen;
};

class ExprNode
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
			FuncCallNode* 	func_call;
		} un;
	};
};

class FuncCallNode
{
public :
	FuncCallNode( char* new_ident, IdentsNode* new_params )
		: 	ident( new_ident ),
			params( new_params )
		{}

	char* 		ident;
	IdentsNode* params;
};

#endif
