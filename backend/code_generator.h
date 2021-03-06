#ifndef _CODE_GENERATOR_H_
#define _CODE_GENERATOR_H_

#include <map>
#include <string>
#include "node.h"
#include "variable.h"
#include "utility.h"

class CodeGenerator
{	
public :

	struct SubSignature
	{
		SubSignature(const std::string& newRetType, const std::string& newSignature)
		{
			retType = newRetType;
			signature = newSignature;
		}

		std::string retType;
		std::string signature;
	};

    CodeGenerator(const char* outputFilePath);
    ~CodeGenerator();

	void Start();
	void End();

	void SubSignatureStart(TypeNode* returnType, const char* subName);
	void SetSubParamDef(TypeNode* typeNode, bool isContinious);
	void SubSignatureEnd();    
	void BlockStart(const char* subName);
	void BlockEnd(const char* subName, const std::map<const char*, Variable, StrCmp>& scopeVariables, bool isNeedRet);

    void LoadIntConst(int num);
    void LoadBoolConst(bool val);
    void LoadStr(const char* str);
    void LoadVariable(const Variable& var);
	void SaveFromStack(const Variable& var);
	void PopFromStack();

	void LogAndOperator();
	void LogOrOperator();
	void LogXorOperator();
	void LogNotOperator();

	void MultOperator();
	void DivOperator();
	void ModOperator();
	void PowOperator(int tmpLocVarInd);

	void NegOperator();
	void NotOperator();
	void AddOperator();
	void SubOperator();

	void LengthOperator();

	void EqOperator();
	void NotEqOperator();
	void LssOperator();
	void GtrOperator();
	void LssEqOperator();
	void GtrEqOperator();

	void PrintInt();
	void PrintChar();
	void PrintBool();
	void PrintString();

	void SetSubCall(const char *subName, int paramsCnt);
	void SetRet();

	void SetLabel(int labelNum);
	void SetLabel(std::string label);
	void SetJumpTo(int toLabelNum);
	void SetJumpTo(std::string label);
	int  SetJumpToNewLabel();
	int  SetCondJumpToNewLabel(bool onTrue);
	void SetCondJumpToLabel(int labelNum, bool onTrue);
	int SetNewLabel();

	void NewArr(TypeNode* type);

	void LoadArrElem(const TypeNode& type);
	void SaveArrElem(const TypeNode& type);
	void LoadArrObj();

	void ExitOn(bool cond);

	void SaveStackDepth();
	void RestoreStackDepth();

	static std::string TypeToString(const TypeNode* node);
    static std::string IntToStr(int num);

private :

	static const std::string ilClassName;
	static const std::string ilAssemblyName;	
	static const std::string TwoTab;
	static const std::string OneTab;
	static const std::string SubCallDecorator;
	static int SubCallDecoratorLength;

    void Reset();

	void IncStackSize();
    void DecStackSize();
	void DecStackSize(int decVal);

	std::string GetNewLabel(int* labelNum);
	std::string GetLabelNameByNum(int labelNum);

	std::map<const char*, SubSignature*, StrCmp> _subsSignatures;

	void SetCondJumpToLabel(std::string label, bool onTrue);	

    FILE*       _output;

	int			_savedStackDepth;
	char*		_currSubName;
	std::string _currSubSig;
	std::string _currSubRetType;
    std::string _ilCode;
    int         _maxStackDepth;
    int         _currStackDepth;
	int         _currLabelNum;    
};

#endif
