L3-Compiler
===========

Замечания:
-1) Не освобождается память из под имен временных переменных.
		const char* varName = GetTmpVarName(_blockLocalsCount);
		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, var));
		AddScopeVar(varName, var);

1) 2 - 3 это
	2 умножть на -3
	из 2 вычесть 3
	как здесь проставить приоритеты непонятно
	--> унарные операции только в скобках


3) Обработка параметров main(в каком виде будет аргумент)

Compiler for L3 language. With Flex/Bison/C++.
