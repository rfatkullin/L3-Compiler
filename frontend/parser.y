%{
#include <stdio.h>
#include <string>
#include "lexer.h"
#include "node.h"
#include "compiler.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"

FILE* out = NULL;
%}

%code requires {
#include "../backend/node.h"
}

%define api.pure
%locations
%lex-param	 {yyscan_t scanner}
%parse-param {yyscan_t scanner}
%parse-param {SubsDefNode** main_node}
%union
{
	char*					m_ident;
	int						m_number;
	char		    		m_ch;
	char*		    		m_str;
	SubsDefNode*			m_subs_def_node;
	SubDefNode*	    		m_sub_def_node;
	SigNode*	    		m_sig_node;
	ParamsDefNode*			m_params_def_node;
	ParamSeqNode*			m_param_sec_node;
	TypeNode*	    		m_type_node;
	StatementsNode*			m_statements_node;
	IdentsNode*	    		m_idents_node;
	StatementNode* 			m_statement_node;
	VarsDefNode* 			m_vars_def_node;
	VarsNode*	    		m_vars_node;
	VarNode*	    		m_var_node;
	AssignNode* 			m_assign_node;
	NewArrNode* 			m_new_arr_node;
	RepeatNode* 			m_repeat_node;
	CheckNode*	    		m_check_node;
	PrintNode*				m_print_node;
	LengthNode*				m_length_node;
	FuncCallNode* 			m_func_call_node;
	ForNode*	    		m_for_node;
	ForFromParamNode* 		m_for_param_node;
	ForToParamNode* 		m_for_to_param_node;
	IfNode*					m_if_node;
	IfSuffixNode* 			m_if_suffix_node;
	ElseIfNode* 			m_else_if_node;
	WhileDoNode* 			m_while_do_node;
	LeftValueNode* 			m_left_value_node;
	ArrElNode*				m_arr_el_node;
	ExprNode*				m_expr_node;
	std::list<ExprNode*>*	m_expr_list;
}

%right ASSIGN
%left PLUS MINUS
%right MULTIPLY DIVIDE MOD
%left FUNC_CALL
%left UMINUS

%token CHAR CHAR_TYPE CHECK PRINT DO ELSE ELSEIF ENDFOR ENDFUNC ENDIF ENDPROC ENDWHILE FF FOR FUNC IF INT_TYPE NIL PROC REPEAT STEP THEN
%token TO TT UNTIL WHILE LPAREN RPAREN RLPAREN RRPAREN COMMA SEMICOLON POINTER_METHOD ASSIGN PLUS CAP EXCL
%token LOG_CAP EQ NOT_EQ LSS_EQ GTR_EQ LSS GTR LOG_AND LOG_OR BOOL_TYPE VOID_TYPE
%token IDENT STR NUMBER BOOL FUNC_CALL_ARR_EL LENGTH
%token VARS_DEF
%token EXPR
%token NEW_ARR
%token ARR_EL
%token UNARY

%type <m_number>			NUMBER
%type <m_ch>				CHAR
%type <m_str>				STR
%type <m_ident>				IDENT
%type <m_subs_def_node> 	start
%type <m_subs_def_node> 	program
%type <m_sub_def_node> 		subprogram_def
%type <m_sig_node>			signature
%type <m_params_def_node>	params_def
%type <m_params_def_node>	params_def_rest
%type <m_param_sec_node> 	param_section
%type <m_type_node> 		type
%type <m_statements_node>	statements
%type <m_idents_node>		idents
%type <m_statement_node>	statement
%type <m_vars_def_node>		vars_def
%type <m_vars_node>			vars
%type <m_var_node>			var
%type <m_assign_node>		assign
%type <m_new_arr_node>		new_arr
%type <m_repeat_node>		repeat
%type <m_check_node>		check
%type <m_print_node>		print
%type <m_length_node>		length
%type <m_func_call_node>	func_call
%type <m_for_node>			for
%type <m_for_param_node>	for_from_param
%type <m_for_to_param_node>	for_to_param
%type <m_if_node>			if_statement
%type <m_if_suffix_node>	if_suffix
%type <m_else_if_node>		else_if
%type <m_while_do_node>		while_do
%type <m_arr_el_node>		get_arr_element
%type <m_number>			basic_type
%type <m_expr_node>			expr
%type <m_expr_node>			term_4
%type <m_expr_node>			term_3
%type <m_expr_node>			term_2
%type <m_expr_node>			term_1
%type <m_expr_node>			pow_factor
%type <m_expr_node>			ufactor
%type <m_expr_node>			factor
%type <m_expr_node>			factor_ident
%type <m_expr_node>			factor_number
%type <m_expr_node>			factor_ch
%type <m_expr_node>			factor_str
%type <m_expr_node>			factor_bool
%type <m_expr_list>			func_params
%type <m_expr_list>			get_arr_indexes
%type <m_expr_list>			func_params_rest

%{
	int yylex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t scanner );
	void yyerror( YYLTYPE *yylloc, yyscan_t scanner, SubsDefNode** main_node,  char* msg );
%}

%%

start : program
	    {
			*main_node = $1;
	    }

program : subprogram_def program
	    {
			$$ = new SubsDefNode($1, $2);
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	| subprogram_def
	    {
			$$ = new SubsDefNode($1, NULL);
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

subprogram_def : FUNC signature POINTER_METHOD type statements ENDFUNC
	    {
		    SubDefNode* node = new SubDefNode(FUNC, $2, $4, $5);
		    $$ = node;
			$$->SetLines(@1.first_line, @6.last_line);
	    }
	| PROC signature statements ENDPROC
	    {
		    SubDefNode* node = new SubDefNode(PROC, $2, new TypeNode(VOID_TYPE, 0), $3);
		    $$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

signature : IDENT LPAREN params_def RPAREN
	    {
			SigNode* node = new SigNode( $1, $3 );
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

params_def : param_section params_def_rest
	    {
			ParamsDefNode* node = new ParamsDefNode($1, $2);
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	|
	    {
			$$ = NULL;
	    }
	;

params_def_rest : SEMICOLON param_section params_def_rest
	    {
			ParamsDefNode* node = new ParamsDefNode($2, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	|
	    {
			$$ = NULL;
	    }
	;

param_section : idents POINTER_METHOD type
	    {
			ParamSeqNode* node = new ParamSeqNode($1, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

idents : IDENT COMMA idents
	    {
			IdentsNode* node = new IdentsNode($1, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| IDENT
	    {
			IdentsNode* node = new IdentsNode($1, NULL);
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

statements : statement SEMICOLON statements
	    {
			StatementsNode* node = new StatementsNode($1, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| statement
	    {
			StatementsNode* node = new StatementsNode($1, NULL);
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	|
	    {
			$$ = NULL;
	    }
	;

statement : vars_def
	    {
			StatementNode* node = new StatementNode(VARS_DEF);
			node->vars_def = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| assign
	    {
			StatementNode* node = new StatementNode( ASSIGN );
			node->assign = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| func_call
	    {
			StatementNode* node = new StatementNode( FUNC_CALL );
			node->func_call = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| if_statement
	    {
			StatementNode* node = new StatementNode( IF );
			node->if_statement = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| while_do
	    {
			StatementNode* node = new StatementNode( WHILE );
			node->while_do = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| for
	    {
			StatementNode* node = new StatementNode( FOR );
			node->for_statement = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| repeat
	    {
			StatementNode* node = new StatementNode( REPEAT );
			node->repeat = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| check
	    {
			StatementNode* node = new StatementNode( CHECK );
			node->check = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| print
	    {
			StatementNode* node = new StatementNode(PRINT);
			node->print = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| length
	    {
			StatementNode* node = new StatementNode(LENGTH);
			node->length = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

check : CHECK expr
	    {
			CheckNode* node = new CheckNode( $2 );
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	;

print : PRINT LPAREN expr RPAREN
	    {
			PrintNode* node = new PrintNode($3);
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

length : LENGTH LPAREN expr RPAREN
	    {
			LengthNode* node = new LengthNode($3);
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

vars_def : vars POINTER_METHOD type
	    {
			VarsDefNode* node = new VarsDefNode( $1, $3 );
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

vars : var COMMA vars
	    {
			VarsNode* node = new VarsNode( $1, $3 );
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| var
	    {
			VarsNode* node = new VarsNode( $1, NULL );
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

var : IDENT
	    {
			VarNode* node = new VarNode( IDENT, $1 );
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| IDENT ASSIGN expr
	    {
			VarNode* node = new VarNode( EXPR, $1 );
			node->expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

assign : IDENT ASSIGN expr
	    {
			LeftValueNode* leftVal = new LeftValueNode(IDENT);
			leftVal->ident = $1;
			AssignNode* node = new AssignNode(leftVal, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| get_arr_element ASSIGN expr
	    {
			LeftValueNode* leftVal = new LeftValueNode(ARR_EL);
			leftVal->arr_el = $1;
			AssignNode* node = new AssignNode(leftVal, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| func_call get_arr_indexes ASSIGN expr
	    {
			LeftValueNode* leftVal = new LeftValueNode(FUNC_CALL_ARR_EL);
			FuncCallGetArrElNode* funcCallGetArrEl = new FuncCallGetArrElNode($1, $2);
			leftVal->funcCallGetArrEl = funcCallGetArrEl;
			AssignNode* node = new AssignNode(leftVal, $4);
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

if_statement : IF expr THEN statements if_suffix ENDIF
	    {
			IfNode* node = new IfNode( $2, $4, $5 );
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

if_suffix : else_if ELSE statements
	    {
			IfSuffixNode* node = new IfSuffixNode($1, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| else_if
	    {
			IfSuffixNode* node = new IfSuffixNode($1, NULL);
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| ELSE statements
	    {
			IfSuffixNode* node = new IfSuffixNode(NULL, $2);
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	|   {
			$$ = NULL;
	    }
	;

else_if : ELSEIF expr THEN statements else_if
	    {
			ElseIfNode* node = new ElseIfNode($2, $4, $5);
			$$ = node;
			$$->SetLines(@1.first_line, @5.last_line);
	    }
	| ELSEIF expr THEN statements
	    {
			ElseIfNode* node = new ElseIfNode($2, $4, NULL);
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

while_do : WHILE expr DO statements ENDWHILE
	    {
			WhileDoNode* node = new WhileDoNode($2, $4);
			$$ = node;
			$$->SetLines(@1.first_line, @5.last_line);
	    }
	;

for : FOR for_from_param TO for_to_param DO statements ENDFOR
	    {
			ForNode* node = new ForNode($2, $4, $6);
			$$ = node;
			$$->SetLines(@1.first_line, @7.last_line);
	    }
	;

for_from_param : vars_def
	    {
			ForFromParamNode* node = new ForFromParamNode(VARS_DEF);
			node->_varsDef = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| assign
	    {
			ForFromParamNode* node = new ForFromParamNode(ASSIGN);
			node->_assign = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

for_to_param : expr
	    {
			ExprNode* expr = new ExprNode();
			expr->op = NUMBER;
			expr->un.num = 1;
			ForToParamNode* node = new ForToParamNode( $1, expr );
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| expr STEP expr
	    {
			ForToParamNode* node = new ForToParamNode( $1, $3 );
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

repeat : REPEAT statements UNTIL expr
	    {
			RepeatNode* node = new RepeatNode( $2, $4 );
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

type : basic_type
	    {
			TypeNode* node = new TypeNode( $1, 0 );
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| RLPAREN type RRPAREN
	    {
			$2->dimen++;
			$$ = $2;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

basic_type : INT_TYPE
	    {
			$$  = INT_TYPE;			
	    }
	| CHAR_TYPE
	    {
			$$ = CHAR_TYPE;			
	    }
	| BOOL_TYPE
	    {
			$$  = BOOL_TYPE;			
	    }
	;

expr : term_4 LOG_OR term_4
	    {
			ExprNode* node = new ExprNode();
			node->op = LOG_OR;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_4 LOG_CAP term_4
	    {
			ExprNode* node = new ExprNode();
			node->op = LOG_CAP;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_4
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

term_4 : term_3 LOG_AND term_3
	    {
			ExprNode* node = new ExprNode();
			node->op = LOG_AND;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_3
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

term_3 : term_2 EQ term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = EQ;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 NOT_EQ term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = NOT_EQ;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 LSS term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = LSS;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 GTR term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = GTR;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 LSS_EQ term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = LSS_EQ;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 GTR_EQ term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = GTR_EQ;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

term_2 : term_2 PLUS term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = PLUS;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_2 MINUS term_2
	    {
			ExprNode* node = new ExprNode();
			node->op = MINUS;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_1
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;


term_1 : term_1 DIVIDE term_1
	    {
			ExprNode* node = new ExprNode();
			node->op = DIVIDE;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_1 MOD term_1
	    {
			ExprNode* node = new ExprNode();
			node->op = MOD;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| term_1 term_1
		{
			ExprNode* node = new ExprNode();
			node->op = MULTIPLY;
			node->bin.left_expr = $1;
			node->bin.right_expr = $2;
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
		}
	| pow_factor
		{
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
		}
	;

pow_factor : pow_factor CAP pow_factor
	    {
			ExprNode* node = new ExprNode();
			node->op = CAP;
			node->bin.left_expr = $1;
			node->bin.right_expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	| ufactor
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

ufactor : LPAREN MINUS factor RPAREN
	    {
			ExprNode* node = new ExprNode();
			node->op = UMINUS;
			node->un.expr = $3;
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	| EXCL factor
	    {
			ExprNode* node = new ExprNode();
			node->op = EXCL;
			node->un.expr = $2;
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	| factor
	    {
			ExprNode* node = new ExprNode();
			node->op = UNARY;
			node->un.expr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

factor : factor_number
	    {
			$$ = $1;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| factor_ch
	    {
			$$ = $1;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| factor_str
	    {
			$$ = $1;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| factor_ident
	    {
			$$ = $1;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| factor_bool
	    {
			$$ = $1;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| get_arr_element
	    {
			ExprNode* node = new ExprNode();
			node->op = ARR_EL;
			node->un.arr_el = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| func_call
	    {
			ExprNode* node = new ExprNode();
			node->op = FUNC_CALL;
			node->un.func_call = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| new_arr
	    {
			ExprNode* node = new ExprNode();
			node->op = NEW_ARR;
			node->un.new_arr = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| length
	    {
			ExprNode* node = new ExprNode();
			node->op = LENGTH;
			node->un.length = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| LPAREN expr RPAREN
	    {
			ExprNode* node = new ExprNode();
			node->op = EXPR;
			node->un.expr = $2;
			$$ = node;
			$$->SetLines(@1.first_line, @3.last_line);
	    }
	;

new_arr : RLPAREN type expr RRPAREN
	    {
			NewArrNode* node = new NewArrNode($2, $3);
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

factor_number : NUMBER
	    {
			ExprNode* node = new ExprNode();
			node->op = NUMBER;
			node->un.num = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

factor_ch : CHAR
	    {
			ExprNode* node = new ExprNode();
			node->op = CHAR;
			node->un.ch = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

factor_str : STR
	    {
			ExprNode* node = new ExprNode();
			node->op = STR;
			node->un.str = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

factor_ident : IDENT
	    {
			ExprNode* node = new ExprNode();
			node->op = IDENT;
			node->un.ident = $1;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;

factor_bool : TT
	    {
			ExprNode* node = new ExprNode();
			node->op = TT;
			node->un.log = true;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	| FF
	    {
			ExprNode* node = new ExprNode();
			node->op = FF;
			node->un.log = false;
			$$ = node;
			$$->SetLines(@1.first_line, @1.last_line);
	    }
	;


get_arr_element : IDENT get_arr_indexes
	    {
			ArrElNode* node = new ArrElNode($1, $2);
			$$ = node;
			$$->SetLines(@1.first_line, @2.last_line);
	    }
	;

get_arr_indexes : RLPAREN expr RRPAREN get_arr_indexes
	    {
			$4->push_back($2);
			$$ = $4;			
	    }
	| RLPAREN expr RRPAREN
		{
			std::list<ExprNode*>* lst = new std::list<ExprNode*>();
			lst->push_back($2);
			$$ = lst;
		}
	;


func_call : IDENT LPAREN func_params RPAREN
	    {
			FuncCallNode* node = new FuncCallNode($1, $3 );
			$$ = node;
			$$->SetLines(@1.first_line, @4.last_line);
	    }
	;

func_params : expr func_params_rest
	    {
			std::list<ExprNode*>* tmpList = $2;
			tmpList->push_front($1);
			$$ = tmpList;			
	    }
	|   {
			$$ = new std::list<ExprNode*>();
	    }
	;

func_params_rest : COMMA expr func_params_rest
	    {
			std::list<ExprNode*>* tmpList = $3;
			tmpList->push_front($2);
			$$ = tmpList;			
	    }
	|
	    {
			$$ = new std::list<ExprNode*>();
	    }
	;
%%

bool ReadFile(const char* inputFilePath, std::string& output)
{
	const int READ_N = 100;
	FILE* pFile = fopen( inputFilePath, "rb" );
	char buff[ READ_N + 1 ];
	int currLen;

	if (pFile == NULL)
	{
	    printf("[Error]: Can't open source file for reading!\n");
	    return false;
	}

	while ( ( currLen = fread( buff, sizeof( char ), READ_N - 1, pFile ) ) > 0 )
	{
	    buff[ currLen ] = 0;
	    output += buff;
	}

	fclose(pFile);

	return true;
}

SubsDefNode* Parse(std::string source)
{
	SubsDefNode* mainNode;
	yyscan_t scanner;
	struct Extra extra;
	init_scanner( const_cast<char*>(source.c_str()), &scanner, &extra );

	int result = yyparse( scanner, &mainNode );
	switch (result)
	{
	    case 0 :
		//Successful
		break;

	    case 1 :
		printf("[Error]: Syntax error!\n");
		break;

	    case 2 :
		printf("[Error]: Memory exhaustion!\n");
		break;

	    default :
		 printf("Unexpected result from yyparse!\n");
	}

	if (result > 0)
		mainNode = NULL;

	destroy_scanner( scanner );

	return mainNode;
}

bool Compile(SubsDefNode* mainNode, const char* outputFilePath)
{
    L3Compiler::Compiler compiler(mainNode, outputFilePath);
    return compiler.Run();
}

int main(int argc, char* argv[])
{
	bool compileRes = false;

    std::string source = "";

    if (!ReadFile(argv[1], source))
    {
	printf("[Error]: Can't read file!\n");
	return 0;
    }

    SubsDefNode* mainNode = Parse(source);

    if (mainNode != NULL)
	compileRes = Compile(mainNode, argv[2]);

    return compileRes == true ? 0 : 1;
}
