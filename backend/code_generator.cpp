#include <sstream>
#include <string.h>
#include <stdlib.h>
#include "node.h"
#include "parser.tab.h"
#include "code_generator.h"

const std::string CodeGenerator::ilClassName = "DummyClass";
const std::string CodeGenerator::ilAssemblyName = "DummyAssembly";
const std::string CodeGenerator :: TwoTab = "\t\t";
const std::string CodeGenerator :: OneTab = "\t";
const std::string CodeGenerator :: SubCallDecorator = "L3_";
int CodeGenerator :: SubCallDecoratorLength;

CodeGenerator :: CodeGenerator(const char* outputFilePath)
{
	_currSubName = NULL;
	SubCallDecoratorLength = SubCallDecorator.length();

	_output = fopen(outputFilePath, "w");

    Reset();
}

void CodeGenerator :: Start()
{
	fprintf(_output, ".assembly %s {}\n", ilAssemblyName.c_str());
	fprintf(_output, ".assembly extern mscorlib {}\n\n");
	fprintf(_output, ".class public %s.%s\n{\n", ilAssemblyName.c_str(), ilClassName.c_str());	

	fprintf(_output,
	".method static int32 main(string[])								\n"
	"{																	\n"
	"		.entrypoint 												\n"
	"		.maxstack 5													\n"
	"		.locals init ( char[][]	V_0, int32 V_1) 					\n"
	" 																	\n"
	"		ldarg.0 													\n"
	"		ldlen 														\n"
	"		conv.i4 													\n"
	"		newarr char[] 												\n"
	"		stloc.0 													\n"
	"		ldc.i4.0 													\n"
	"		stloc.1 													\n"
	"		br IL_001f 													\n"
	"	IL_0010:  ldloc.0 												\n"
	"		ldloc.1 													\n"
	"		ldarg.0 													\n"
	"		ldloc.1 													\n"
	"		ldelem.ref													\n"
	"		callvirt instance char[] string::ToCharArray()				\n"
	"		stelem.ref	 												\n"
	"		ldloc.1 													\n"
	"		ldc.i4.1 													\n"
	"		add 														\n"
	"		stloc.1 													\n"
	"	IL_001f:  ldloc.1 												\n"
	"		ldarg.0 													\n"
	"		ldlen 														\n"
	"		conv.i4 													\n"
	"		blt IL_0010 												\n"
	"		ldloc.0 													\n"
	"		call int32 DummyAssembly.DummyClass::%smain(char[][])		\n"
	"		ret															\n"
	"} // main															\n",
	SubCallDecorator.c_str());
}

void CodeGenerator :: End()
{
	fprintf(_output, "}\n");
}

CodeGenerator :: ~CodeGenerator()
{
	if (_currSubName != NULL)
		delete [] _currSubName;

    fclose(_output);
}

void CodeGenerator :: Reset()
{
    _ilCode = "";
	_maxStackDepth  = 0;
    _currStackDepth = 0;
	_currLabelNum   = 0;    
}

void CodeGenerator :: SubSignatureStart(TypeNode* returnType)
{
	Reset();

	_currSubRetType = TypeToString(returnType);
}

void CodeGenerator :: SetSubName(const char* name)
{
	_currSubName = new char [strlen(name) + SubCallDecorator.length() + 1];
	strcpy(_currSubName, SubCallDecorator.c_str());
	strcpy(_currSubName + SubCallDecoratorLength, name);

	_currSubSig = std::string(_currSubName) + "(";
}

void CodeGenerator :: SetSubParamDef(TypeNode* typeNode, bool isContinious)
{
	_currSubSig += TypeToString(typeNode) + (isContinious ? ", " : "");
}

void CodeGenerator :: SubSignatureEnd()
{
	_currSubSig += ")";

	_subsFullName.insert(std::make_pair(_currSubName, _currSubRetType + " " + ilAssemblyName + "." + ilClassName + "::" + _currSubSig));	

	fprintf(_output, ".method static %s %s\n", _currSubRetType.c_str(), _currSubSig.c_str());
}

void CodeGenerator :: BlockStart()
{	
    fprintf(_output, "{\n");
}

void CodeGenerator :: BlockEnd(const char* subName, const std::map<const char*, Variable, StrCmp>& scopeVariables,bool isNeedRet)
{    
	fprintf(_output, "%s.maxstack %d\n", TwoTab.c_str(), _maxStackDepth);

	std::string localsInit = TwoTab + ".locals init(";    
	std::list<Variable> locals;
    for (std::map<const char*, Variable, StrCmp>::const_iterator it = scopeVariables.begin(); it != scopeVariables.end(); ++it)
    {
		if (it->second._isArg)
            continue;

		locals.push_back(it->second);
	}

	locals.sort(Variable::VarCompare);
	std::list<Variable>::iterator lastElem = --locals.end();
	for (std::list<Variable>::iterator it = locals.begin(); it != locals.end(); ++it)
	{
		localsInit += TypeToString(it->_type);

		if (it != lastElem)
            localsInit += ",";
    }
	localsInit += ")";

	fprintf(_output, "%s\n\n", localsInit.c_str() );

    fprintf(_output, "%s", _ilCode.c_str() );

	fprintf(_output, "%s%s\n} //%s\n\n", TwoTab.c_str(), isNeedRet ? "ret" : "", subName);
}

void CodeGenerator :: LoadIntConst(int num)
{
    IncStackSize();
	_ilCode += TwoTab + "ldc.i4\t" + IntToStr(num) + "\n";
}

void CodeGenerator :: LoadBoolConst(bool val)
{
    IncStackSize();

	_ilCode += TwoTab;

    if (val)
        _ilCode += "ldc.i4.1\n";
    else
        _ilCode += "ldc.i4.0\n";
}

void CodeGenerator :: LoadStr(const char* str)
{
    IncStackSize();

	_ilCode += TwoTab + "ldstr \"" + str + "\"\n";
	_ilCode += TwoTab + "callvirt instance char[] string::ToCharArray()\n";
}

void CodeGenerator :: LoadVariable(const Variable& var)
{
    IncStackSize();

	_ilCode += TwoTab;

	if (var._isArg)
		_ilCode = _ilCode + "ldarg " + IntToStr(var._id) + "\n";
    else
		_ilCode = _ilCode + "ldloc " + IntToStr(var._id) + "\n";
}

void CodeGenerator :: SaveFromStack(const Variable& var)
{
    DecStackSize();

	_ilCode += TwoTab;

	if (var._isArg)
		_ilCode = _ilCode + "starg " + IntToStr(var._id) + "\n";
    else
		_ilCode = _ilCode + "stloc " + IntToStr(var._id) + "\n";

}

void CodeGenerator :: PopFromStack()
{
	_ilCode += TwoTab + "pop\n";
	DecStackSize();
}

void CodeGenerator :: IncStackSize()
{
    _maxStackDepth = std::max(_maxStackDepth, ++_currStackDepth);
}

void CodeGenerator :: DecStackSize()
{
    --_currStackDepth;
}

void CodeGenerator :: SaveStackDepth()
{
	_savedStackDepth = _currStackDepth;
}

void CodeGenerator :: RestoreStackDepth()
{
	_currStackDepth = _savedStackDepth;
}

void CodeGenerator :: LogAndOperator()
{
	std::string label = GetNewLabel(NULL);

	_ilCode += TwoTab + "brtrue.s " + label + "\n";
	_ilCode += TwoTab + "pop\n";
	_ilCode += TwoTab + "ldc.i4.0\n";
	_ilCode += OneTab + label + ": nop\n";

	DecStackSize();
}

void CodeGenerator :: LogOrOperator()
{
	std::string label = GetNewLabel(NULL);

	_ilCode += TwoTab + "brfalse.s " + label + "\n";
	_ilCode += TwoTab + "pop\n";
	_ilCode += TwoTab + "ldc.i4.1\n";
	_ilCode += OneTab + label + ": nop\n";

	DecStackSize();
}

void CodeGenerator :: LogXorOperator()
{
	_ilCode += TwoTab + "xor\n";
	DecStackSize();
}

void CodeGenerator :: LogNotOperator()
{
	_ilCode += TwoTab + "ldc.i4.0\n";
	_ilCode += TwoTab + "ceq\n";

	DecStackSize();
}

void CodeGenerator :: MultOperator()
{
	_ilCode += TwoTab + "mul\n";
	DecStackSize();
}

void CodeGenerator :: DivOperator()
{
	_ilCode += TwoTab + "div\n";
	DecStackSize();
}

void CodeGenerator :: ModOperator()
{
	_ilCode += TwoTab + "rem\n";
	DecStackSize();
}

void CodeGenerator :: PowOperator(int tmpLocVarInd)
{
	std::string varNum = IntToStr(tmpLocVarInd);
	_ilCode += TwoTab + "stloc.s " + varNum + "\n";
	_ilCode += TwoTab + "conv.r4\n";
	_ilCode += TwoTab + "ldloc.s " + varNum + "\n";
	_ilCode += TwoTab + "conv.r4\n";
	_ilCode += TwoTab + "call System.Math.Pow\n";

	DecStackSize();
}

void CodeGenerator :: NegOperator()
{
	_ilCode += TwoTab + "conv.i4\n";
	_ilCode += TwoTab + "neg\n";
}

void CodeGenerator :: NotOperator()
{
	_ilCode += TwoTab + "not\n";
}

void CodeGenerator :: AddOperator()
{
	_ilCode += TwoTab + "add\n";

	DecStackSize();
}

void CodeGenerator :: SubOperator()
{
	_ilCode += TwoTab + "sub\n";

	DecStackSize();
}

void CodeGenerator :: LengthOperator()
{
	_ilCode += TwoTab + "ldlen\n";

	DecStackSize();
}

void CodeGenerator :: EqOperator()
{
	_ilCode += TwoTab + "ceq\n";

	DecStackSize();
}

void CodeGenerator :: NotEqOperator()
{
	_ilCode += TwoTab + "ceq\n";
	_ilCode += TwoTab + "ldc.i4.0\n";
	_ilCode += TwoTab + "ceq\n";	

	DecStackSize();
}

void CodeGenerator :: LssOperator()
{
	_ilCode += TwoTab + "clt\n";

	DecStackSize();
}

void CodeGenerator :: GtrOperator()
{
	_ilCode += TwoTab + "cgt\n";

	DecStackSize();
}

void CodeGenerator :: LssEqOperator()
{
	_ilCode += TwoTab + "cgt\n";
	_ilCode += TwoTab + "ldc.i4.0\n";
	_ilCode += TwoTab + "ceq\n";

	DecStackSize();
}

void CodeGenerator :: GtrEqOperator()
{
	_ilCode += TwoTab + "clt\n";
	_ilCode += TwoTab + "ldc.i4.0\n";
	_ilCode += TwoTab + "ceq\n";

	DecStackSize();
}

void CodeGenerator :: PrintInt()
{
	_ilCode += TwoTab + "call void [mscorlib]System.Console::WriteLine(int32)\n";

	DecStackSize();
}

void CodeGenerator :: PrintChar()
{
	_ilCode += TwoTab + "call void [mscorlib]System.Console::WriteLine(char)\n";

	DecStackSize();
}

void CodeGenerator :: PrintBool()
{
	_ilCode += TwoTab + "call void [mscorlib]System.Console::WriteLine(bool)\n";

	DecStackSize();
}

void CodeGenerator :: PrintString()
{
	_ilCode += TwoTab + "call void [mscorlib]System.Console::WriteLine(char[])\n";

	DecStackSize();
}

void CodeGenerator :: SetRet()
{
	_ilCode += TwoTab + "ret\n";
}

void CodeGenerator :: SetSubCall(const char* subName)
{
	_ilCode += TwoTab + "call " + _subsFullName[(SubCallDecorator + subName).c_str()] + "\n";
}

int CodeGenerator :: SetNewLabel()
{
	int labelNum;
	std::string label = GetNewLabel(&labelNum);

	_ilCode += OneTab + label + ": nop\n";

	return labelNum;
}

int CodeGenerator :: SetCondJumpToNewLabel(bool onTrue)
{
	int labelNum;	

	SetCondJumpToLabel(GetNewLabel(&labelNum), onTrue);

	return labelNum;
}

void CodeGenerator :: SetCondJumpToLabel(int labelNum, bool onTrue)
{
	std::string label = GetLabelNameByNum(labelNum);

	SetCondJumpToLabel(label, onTrue);
}

void CodeGenerator :: SetCondJumpToLabel(std::string label, bool onTrue)
{
	_ilCode += TwoTab;

	if (onTrue)
	{
		_ilCode += "brtrue ";
	}
	else
	{
		_ilCode += "brfalse ";
	}

	_ilCode += label + "\n";

	DecStackSize();
}

void CodeGenerator :: NewArr(TypeNode* type)
{
	_ilCode += TwoTab + "newarr " + TypeToString(type) + "\n";
}

void CodeGenerator :: LoadArrElem(const TypeNode& type)
{
	_ilCode += TwoTab;

	if (type.dimen == 0)
		_ilCode += "ldelem " + TypeToString(&type);
	else
		_ilCode += "ldelem.ref";

	_ilCode += "\n";
}

void CodeGenerator :: LoadArrObj()
{
	_ilCode += TwoTab + "ldelem.ref\n";
}

void CodeGenerator :: SaveArrElem(const TypeNode& type)
{
	_ilCode += TwoTab;

	if (type.dimen == 0)
		_ilCode += "stelem " + TypeToString(&type);
	else
		_ilCode += "stelem.ref";

	_ilCode += "\n";
}

void SaveArrElem(const TypeNode& type);

void CodeGenerator :: ExitOn(bool cond)
{
	int labelNum;
	std::string label = GetNewLabel(&labelNum);

	SetCondJumpToLabel(label, !cond);

	_ilCode += TwoTab + "ldc.i4.1\n";
	_ilCode += TwoTab + "call void [mscorlib]System.Environment::Exit(int32)\n";

	SetLabel(label);
}

void CodeGenerator :: SetJumpTo(int toLabelNum)
{
	_ilCode += TwoTab + "br " + GetLabelNameByNum(toLabelNum) + "\n";
}

void CodeGenerator :: SetJumpTo(std::string label)
{
	_ilCode += TwoTab + "br " + label + "\n";
}

int CodeGenerator :: SetJumpToNewLabel()
{
	int labelNum;
	std::string label = GetNewLabel(&labelNum);

	SetJumpTo(label);

	return labelNum;
}

void CodeGenerator :: SetLabel(int labelNum)
{
	_ilCode += OneTab + GetLabelNameByNum(labelNum) + ": nop\n";
}

void CodeGenerator :: SetLabel(std::string label)
{
	_ilCode += OneTab + label + ": nop\n";
}

std::string CodeGenerator :: TypeToString(const TypeNode* node)
{
    std::string str = "";

	switch (node->type)
    {
		case INT_TYPE :
            str += "int32";
            break;
		case BOOL_TYPE :
            str += "bool";
            break;
		case CHAR_TYPE :
            str += "char";
            break;
		case VOID_TYPE :
			str += "void";
			break;
		default :
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

std::string CodeGenerator :: GetNewLabel(int* labelNum)
{
	if (labelNum != NULL)
		*labelNum = _currLabelNum;

	return GetLabelNameByNum(_currLabelNum++);
}

std::string CodeGenerator :: GetLabelNameByNum(int labelNum)
{
	return "Label" + IntToStr(labelNum);
}
