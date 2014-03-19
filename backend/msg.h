#ifndef _MSG_H_
#define _MSG_H_

namespace L3Compiler
{
	class Msg
	{
	public :
		static const int   ErrorStrCnt = 4;
		static const char* ErrorsStrList[ErrorStrCnt];

		static const int VariableNotDiclared;
		static const int TypeMismatch;
		static const int UnexpectedOperator;
		static const int PrintTypeMismatch;
	};

} // L3Compiler namespace

#endif
