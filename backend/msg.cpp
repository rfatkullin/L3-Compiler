#include "msg.h"

namespace L3Compiler
{
	const char* Msg::ErrorsStrList[ErrorStrCnt] =
	{
		"Variable not declared in this scope!",
		"Type mismatch!",
		"Unexpected operator!",
		"Operator print take only operands of int, char, bool types!"
	};

	const int Msg::VariableNotDiclared	= 0;
	const int Msg::TypeMismatch			= 1;
	const int Msg::UnexpectedOperator	= 2;
	const int Msg::PrintTypeMismatch	= 3;


} // L3Compiler namespace
