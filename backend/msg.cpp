#include "msg.h"

namespace L3Compiler
{
	const char* Msg::ErrorsStrList[ErrorStrCnt] =
	{
		"Variable not declared in this scope!",
		"Type mismatch!",
		"Unexpected operator!",
		"Print operator takes only operands of int, char, bool and char[] types!",
		"Conflicting in declaration function locals!",
		"Expected identificator!",
		"For loop do not declare more than one variable!",
		"For loop variable must be int or char type!",
		"Main func must return int value!",
		"Main func must take only one param with string array type!",
		"Function return type doesn't match function signature type!",
		"Unknown sub call",
		"Sub params types dismatch!",
		"Too few arguments to function",
		"Too many arguments to function",
		"Operands of boolean operator must be boolean!\n",
		"Operands of pow, divide, mod and multiply operators must be boolean!",
		"Minus operator takes next args - (int, int), (char, char) or (char, int)!",
		"Add operator takes next args - (int, int), (int, char) or (char, int)!",
		"Equality operator takes next args -(type, type), (int, char), (char, int)!",
		"Compare operator takes next args -(int, int), (char, char), (int, char), (char, int)!",
		"Length operator takes operand of array type!",
		"Main function is not defined!"
	};

	const int Msg::VariableNotDiclared				= 0;
	const int Msg::TypeMismatch						= 1;
	const int Msg::UnexpectedOperator				= 2;
	const int Msg::PrintOperatorBadArg				= 3;
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
	const int Msg::BoolOperatorsBadArgs				= 15;
	const int Msg::ArithmeticOperatorsBadArgs		= 16;
	const int Msg::MinusOperatorBadArgs				= 17;
	const int Msg::AddOperatorBadArgs				= 18;
	const int Msg::EqOperatorBadArgs				= 19;
	const int Msg::CompareOperatorBadArgs			= 20;
	const int Msg::LengthOperatorBadArg				= 21;
	const int Msg::MainFuncIsNotDefined					= 22;

} // L3Compiler namespace
