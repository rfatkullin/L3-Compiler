#ifndef _MSG_H_
#define _MSG_H_

namespace L3Compiler
{
	class Msg
	{
	public :
		static const int   ErrorStrCnt = 24;
		static const char* ErrorsStrList[ErrorStrCnt];

		static const int VariableNotDiclared;
		static const int TypeMismatch;
		static const int UnexpectedOperator;
		static const int PrintOperatorBadArg;
		static const int DeclarationConflict;
		static const int IdentificatorExpected;
		static const int ForLoopDefOnlyOneVar;
		static const int ForLoopVariableMustBeIntOrChar;
		static const int MainFuncMustReturnIntValue;
		static const int MainFuncTakeOneStringArrParam;
		static const int SubSigReturnMismatch;
		static const int UnknownSubCall;
		static const int SubParamTypeDismatch;
		static const int TooFewArgs;
		static const int TooManyArgs;
		static const int BoolOperatorsBadArgs;
		static const int ArithmeticOperatorsBadArgs;
		static const int SubOperatorBadArgs;
		static const int AddOperatorBadArgs;
		static const int EqOperatorBadArgs;
		static const int CompareOperatorBadArgs;
		static const int LengthOperatorBadArg;
		static const int MainFuncIsNotDefined;
		static const int UnaryMinusBadArgs;

	};

} // L3Compiler namespace

#endif
