#include "msg.h"

namespace L3Compiler
{
	const char* Msg::ErrorsStrList[ErrorStrCnt] =
	{
		"Variable not declared in this scope!",
		"Type mismatch!",
		"Unexpected operator!",
		"Operator print takes only operands of int, char, bool types!",
		"Conflicting in declaration function locals!",
		"Expected identificator!",
		"For loop do not declare more than one variable!",
		"For loop variable must be int or char type!",
		"Main func must return int value!",
		"Main func must take only one param with string array type!",
		"Function return type doesn't match function signature type!",
		"Unknown sub call",
		"[Набор слов:)]Sub params types dismatch!",
		"Too few arguments to function",
		"Too many arguments to function"
	};

	const int Msg::VariableNotDiclared				= 0;
	const int Msg::TypeMismatch						= 1;
	const int Msg::UnexpectedOperator				= 2;
	const int Msg::PrintTypeMismatch				= 3;
	const int Msg::DeclarationConflict				= 4;
	const int Msg::IdentificatorExpected			= 5;
	const int Msg::ForLoopDefOnlyOneVar				= 6;
	const int Msg::ForLoopVariableMustBeIntOrChar	= 7;
	const int Msg::MainFuncMustReturnIntValue		= 8;
	const int Msg::MainFuncTakeOneStringArrParam	= 9;
	const int Msg::SubSigReturnMismatch				= 10;
	const int Msg::UnknownSubCall					= 11;
	const int Msg::SubParamTypeDismatch				= 12;
	const int Msg::TooFewArgs						= 13;
	const int Msg::TooManyArgs						= 14;

} // L3Compiler namespace
