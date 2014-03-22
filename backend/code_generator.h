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
    CodeGenerator(const char* outputFilePath);
    ~CodeGenerator();

    void SubDef(TypeNode* returnType);
    void SubDef();
    void SubName(const char* name);
    void SignatureStart();
    void Def(const std::string& typeStr, bool isContinious);
    void SignatureEnd();
    void MarkAsEntryPoint();
    void BlockStart();
    void BlockEnd(const std::map<const char*, Variable, StrCmp>& scopeVariables);

    void LoadIntConst(int num);
    void LoadBoolConst(bool val);
    void LoadStr(const char* str);
    void LoadVariable(const Variable& var);
	void SaveFromStack(const Variable& var);

	void LogAndOperator();
	void LogOrOperator();
	void LogXorOperator();
	void LogNotOperator();

	void DivOperator();
	void ModOperator();
	void PowOperator(int tmpLocVarInd);

	void NegOperator();
	void NotOperator();
	void Add();
	void Sub();

	void EqOperator();
	void NotEqOperator();
	void LssOperator();
	void GtrOperator();
	void LssEqOperator();
	void GtrEqOperator();

	void PrintInt();
	void PrintChar();
	void PrintBool();

	void SetLabel(int labelNum);
	void SetLabel(std::string label);
	void SetJumpTo(int toLabelNum);
	int SetCondJumpToNewLabel(bool onTrue);
	void CondJumpToLabel(int labelNum, bool onTrue);
	int SetNewLabel();

	void ExitOn(bool cond);

    static std::string TypeToString(TypeNode* node);
    static std::string IntToStr(int num);

private :

	static const std::string TwoTab;
	static const std::string OneTab;

    void Reset();

	void IncStackSize();
    void DecStackSize();

	std::string GetNewLabel(int* labelNum);
	std::string GetLabelNameByNum(int labelNum);

	void CondJumpToLabel(std::string label, bool onTrue);

    FILE*       _output;

    std::string _ilCode;
    int         _maxStackDepth;
    int         _currStackDepth;
	int         _currLabelNum;
    bool        _isEntryPoint;
};

#endif
