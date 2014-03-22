#include "msg.h"

namespace L3Compiler
{
	const char* Msg::ErrorsStrList[ErrorStrCnt] =
	{
		"Variable not declared in this scope!",
		"Type mismatch!",
		"Unexpected operator!",
		"Operator print take only operands of int, char, bool types!",
		"Conflicting in declaration function locals!",
		"Expected identificator!",
		"For loop do not declare more than one variable!",
		"For loop variable must be int or char type!",
	};

	const int Msg::VariableNotDiclared				= 0;
	const int Msg::TypeMismatch						= 1;
	const int Msg::UnexpectedOperator				= 2;
	const int Msg::PrintTypeMismatch				= 3;
	const int Msg::DeclarationConflict				= 4;
	const int Msg::IdentificatorExpected			= 5;
	const int Msg::ForLoopDefOnlyOneVar				= 6;
	const int Msg::ForLoopVariableMustBeIntOrChar	= 7;


} // L3Compiler namespace
