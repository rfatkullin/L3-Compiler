#include <sstream>
#include <string.h>
#include "node.h"
#include "parser.tab.h"
#include "code_generator.h"

const std::string CodeGenerator :: InstPrefix = "\t\t";

CodeGenerator :: CodeGenerator(const char* outputFilePath)
{
    _output = fopen(outputFilePath, "w");

    fprintf(_output, ".assembly AzazaAssembly {}\n");
    fprintf(_output, ".assembly extern mscorlib {}\n\n");

    Reset();
}

CodeGenerator :: ~CodeGenerator()
{
    fclose(_output);
}

void CodeGenerator :: Reset()
{
    _ilCode = "";
    _maxStackDepth = -1;
    _currStackDepth = 0;
    _isEntryPoint = false;
}

void CodeGenerator :: SubDef(TypeNode* returnType)
{
    fprintf(_output, ".method static %s ", TypeToString(returnType).c_str());

    Reset();
}

void CodeGenerator :: SubDef()
{
    fprintf(_output, ".method static void ");
}

void CodeGenerator :: SubName(const char* name)
{
    fprintf(_output, "%s", name);

    if (!strcmp(name, "main"))
        _isEntryPoint = true;
}

void CodeGenerator :: SignatureStart()
{
    fprintf(_output, "(");
}

void CodeGenerator :: Def(const std::string& typeStr, bool isContinious)
{
    if (isContinious)
        fprintf(_output, "%s, ", typeStr.c_str());
    else
        fprintf(_output, "%s", typeStr.c_str());
}

void CodeGenerator :: SignatureEnd()
{
    fprintf(_output, ")");
}

void CodeGenerator :: MarkAsEntryPoint()
{
    fprintf(_output, ".entrypoint\n");
}

void CodeGenerator :: BlockStart()
{
    fprintf(_output, "{\n");
}

void CodeGenerator :: BlockEnd(const std::map<const char*, Variable, StrCmp>& scopeVariables)
{
    if (_isEntryPoint)
        fprintf(_output, "%s.entrypoint\n", InstPrefix.c_str() );

    fprintf(_output, "%s.maxstack %d\n", InstPrefix.c_str(), _maxStackDepth);

    std::string localsInit = InstPrefix + ".locals init(";
    int varIndex = 0;
    int varsCnt = scopeVariables.size();
    for (std::map<const char*, Variable, StrCmp>::const_iterator it = scopeVariables.begin(); it != scopeVariables.end(); ++it)
    {
        if (it->second._isArg)
            continue;

        ++varIndex;
        localsInit += TypeToString(it->second._type);

        if (varIndex < varsCnt)
            localsInit += ",";

    }
    localsInit += ")\n\n";

    fprintf(_output, "%s\n", localsInit.c_str() );

    fprintf(_output, "%s", _ilCode.c_str() );

    fprintf(_output, "%sret\n}\n\n", InstPrefix.c_str());
}

void CodeGenerator :: LoadIntConst(int num)
{
    IncStackSize();
    _ilCode += InstPrefix + "ldc.i4\t" + IntToStr(num) + "\n";
}

void CodeGenerator :: LoadBoolConst(bool val)
{
    IncStackSize();

    _ilCode += InstPrefix;

    if (val)
        _ilCode += "ldc.i4.1\n";
    else
        _ilCode += "ldc.i4.0\n";
}

void CodeGenerator :: LoadStr(const char* str)
{
    IncStackSize();

    _ilCode = _ilCode + "ldstr " + str + "\n";
}

void CodeGenerator :: LoadVariable(const Variable& var)
{
    IncStackSize();

    _ilCode += InstPrefix;

    if (var._isArg)
        _ilCode = _ilCode + "ldarg " + IntToStr(var._id) + "\n";
    else
        _ilCode = _ilCode + "ldloc " + IntToStr(var._id) + "\n";
}

void CodeGenerator :: SaveFromStack(const Variable& var)
{
    DecStackSize();

    _ilCode += InstPrefix;

    if (var._isArg)
        _ilCode = _ilCode + "starg " + IntToStr(var._id) + "\n";
    else
        _ilCode = _ilCode + "stloc " + IntToStr(var._id) + "\n";

}

void CodeGenerator :: IncStackSize()
{
    _maxStackDepth = std::max(_maxStackDepth, ++_currStackDepth);
}

void CodeGenerator :: DecStackSize()
{
    --_currStackDepth;
}

std::string CodeGenerator :: TypeToString(TypeNode* node)
{
    std::string str = "";

    switch (node->type)
    {
        case INT_TYPE:
            str += "int32";
            break;
        case BOOL_TYPE:
            str += "bool";
            break;
        case CHAR_TYPE:
            str += "char";
            break;
    }

    for (int i = 0; i < node->dimen; ++i)
        str += "[]";

    return str;
}

std::string CodeGenerator :: IntToStr(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}
