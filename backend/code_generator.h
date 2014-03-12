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

    static std::string TypeToString(TypeNode* node);
    static std::string IntToStr(int num);

private :

    static const std::string InstPrefix;

    void Reset();
    void IncStackSize();
    void DecStackSize();

    FILE*       _output;

    std::string _ilCode;
    int         _maxStackDepth;
    int         _currStackDepth;
    bool        _isEntryPoint;
};

#endif
