#ifndef _MSG_H_
#define _MSG_H_

namespace L3Compiler
{
	class Msg
	{
	public :
		static const int   ErrorStrCnt = 15;
		static const char* ErrorsStrList[ErrorStrCnt];

		static const int VariableNotDiclared;
		static const int TypeMismatch;
		static const int UnexpectedOperator;
		static const int PrintTypeMismatch;
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
	};

} // L3Compiler namespace

#endif
